//
// Created by stephane bourque on 2022-10-25.
//

#include <mutex>

#include "Poco/NObserver.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Logger.h"

#include "framework/UI_WebSocketClientServer.h"
#include "framework/AuthClient.h"
#include "framework/MicroServiceFuncs.h"

#include "fmt/format.h"

#define DBG { std::cout << __LINE__ << std::endl; }

namespace OpenWifi {

	void UI_WebSocketClientServer::NewClient(Poco::Net::WebSocket & WS, const std::string &Id, const std::string &UserName ) {

        std::lock_guard G(Mutex_);
        auto Client = std::make_unique<UI_WebSocketClientInfo>(WS,Id, UserName);
        auto ClientSocket = Client->WS_->impl()->sockfd();

        Client->WS_->setNoDelay(true);
        Client->WS_->setKeepAlive(true);
        Client->WS_->setBlocking(false);
        Reactor_.addEventHandler(*Client->WS_,
                                 Poco::NObserver<UI_WebSocketClientServer, Poco::Net::ReadableNotification>(
                                         *this, &UI_WebSocketClientServer::OnSocketReadable));
        Reactor_.addEventHandler(*Client->WS_,
                                 Poco::NObserver<UI_WebSocketClientServer, Poco::Net::ShutdownNotification>(
                                         *this, &UI_WebSocketClientServer::OnSocketShutdown));
        Reactor_.addEventHandler(*Client->WS_,
                                 Poco::NObserver<UI_WebSocketClientServer, Poco::Net::ErrorNotification>(
                                         *this, &UI_WebSocketClientServer::OnSocketError));
        Client->SocketRegistered_ = true;
        Clients_[ClientSocket] = std::move(Client);
    }

	void UI_WebSocketClientServer::SetProcessor( UI_WebSocketClientProcessor * F) {
		Processor_ = F;
	}

	UI_WebSocketClientServer::UI_WebSocketClientServer() noexcept:
		 SubSystemServer("WebSocketClientServer", "UI-WSCLNT-SVR", "websocketclients")
	{
	}

    void UI_WebSocketClientServer::EndConnection([[maybe_unused]]  std::lock_guard<std::recursive_mutex> &G, ClientList::iterator &Client) {
        if(Client->second->SocketRegistered_) {
            Client->second->SocketRegistered_ = false;
            (*Client->second->WS_).shutdown();
            Reactor_.removeEventHandler(*Client->second->WS_,
                                        Poco::NObserver<UI_WebSocketClientServer,
                                                Poco::Net::ReadableNotification>(*this,&UI_WebSocketClientServer::OnSocketReadable));
            Reactor_.removeEventHandler(*Client->second->WS_,
                                        Poco::NObserver<UI_WebSocketClientServer,
                                                Poco::Net::ShutdownNotification>(*this,&UI_WebSocketClientServer::OnSocketShutdown));
            Reactor_.removeEventHandler(*Client->second->WS_,
                                        Poco::NObserver<UI_WebSocketClientServer,
                                                Poco::Net::ErrorNotification>(*this,&UI_WebSocketClientServer::OnSocketError));
        }
        Clients_.erase(Client);
        std::cout << "How many clients: " << Clients_.size() << std::endl;
    }

	void UI_WebSocketClientServer::run() {
		Running_ = true ;
		Utils::SetThreadName("ws:uiclnt-svr");
		while(Running_) {
			Poco::Thread::trySleep(2000);

			if(!Running_)
				break;
		}
	};

	int UI_WebSocketClientServer::Start() {
        poco_information(Logger(),"Starting...");
		GoogleApiKey_ = MicroServiceConfigGetString("google.apikey","");
		GeoCodeEnabled_ = !GoogleApiKey_.empty();
		ReactorThread_.start(Reactor_);
		Thr_.start(*this);
		return 0;
	};

	void UI_WebSocketClientServer::Stop() {
		if(Running_) {
            poco_information(Logger(),"Stopping...");
            Clients_.clear();
			Reactor_.stop();
			ReactorThread_.join();
			Running_ = false;
			Thr_.wakeUp();
			Thr_.join();
            poco_information(Logger(),"Stopped...");
		}
	};

	bool UI_WebSocketClientServer::SendToId(const std::string &Id, const std::string &Payload) {
		std::lock_guard G(Mutex_);

        for(const auto &Client:Clients_) {
            if(Client.second->Id_==Id)
                return Client.second->WS_->sendFrame(Payload.c_str(),(int)Payload.size());
        }
		return false;
	}

	bool UI_WebSocketClientServer::SendToUser(const std::string &UserName, const std::string &Payload) {
		std::lock_guard G(Mutex_);
		uint64_t Sent=0;

		for(const auto &client:Clients_) {
			if(client.second->UserName_ == UserName) {
				try {
					if (client.second->WS_->sendFrame(Payload.c_str(),(int)Payload.size()))
						Sent++;
				} catch (...) {
					return false;
				}
			}
		}
		return Sent>0;
	}

	void UI_WebSocketClientServer::SendToAll(const std::string &Payload) {
		std::lock_guard G(Mutex_);

		for(const auto &client:Clients_) {
			try {
				client.second->WS_->sendFrame(Payload.c_str(),(int)Payload.size());
			} catch (...) {

			}
		}
	}

    UI_WebSocketClientServer::ClientList::iterator UI_WebSocketClientServer::FindWSClient( [[maybe_unused]]  std::lock_guard<std::recursive_mutex> &G, int ClientSocket) {
        return Clients_.find(ClientSocket);
    }

    void UI_WebSocketClientServer::OnSocketError([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ErrorNotification> &pNf) {
        std::lock_guard     G(LocalMutex_);
        auto Client = FindWSClient(G,pNf->socket().impl()->sockfd());
        if(Client==end(Clients_))
            return;
        EndConnection(G,Client);
	}

	void UI_WebSocketClientServer::OnSocketReadable([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ReadableNotification> &pNf) {

        UI_WebSocketClientServer::ClientList::iterator Client;

        std::lock_guard     G(LocalMutex_);

		try {

            Client = FindWSClient(G,pNf->socket().impl()->sockfd());
            if( Client == end(Clients_))
                return;

			Poco::Buffer<char> IncomingFrame(0);
            int flags;
            int n;
			n = Client->second->WS_->receiveFrame(IncomingFrame, flags);
			auto Op = flags & Poco::Net::WebSocket::FRAME_OP_BITMASK;

			if (n == 0) {
				poco_debug(Logger(),fmt::format("CLOSE({}): {} UI Client is closing WS connection.", Client->second->Id_, Client->second->UserName_));
				return EndConnection(G, Client);
			}

			switch (Op) {
			case Poco::Net::WebSocket::FRAME_OP_PING: {
                Client->second->WS_->sendFrame("", 0,
							   (int)Poco::Net::WebSocket::FRAME_OP_PONG |
								   (int)Poco::Net::WebSocket::FRAME_FLAG_FIN);
			} break;
			case Poco::Net::WebSocket::FRAME_OP_PONG: {
			} break;
			case Poco::Net::WebSocket::FRAME_OP_CLOSE: {
				poco_debug(Logger(),fmt::format("CLOSE({}): {} UI Client is closing WS connection.", Client->second->Id_, Client->second->UserName_));
                return EndConnection(G, Client);
			} break;
			case Poco::Net::WebSocket::FRAME_OP_TEXT: {
				IncomingFrame.append(0);
				if (!Client->second->Authenticated_) {
					std::string Frame{IncomingFrame.begin()};
					auto Tokens = Utils::Split(Frame, ':');
					bool Expired = false, Contacted = false;
					if (Tokens.size() == 2 &&
						AuthClient()->IsAuthorized(Tokens[1], Client->second->UserInfo_, 0, Expired, Contacted)) {
                        Client->second->Authenticated_ = true;
                        Client->second->UserName_ = Client->second->UserInfo_.userinfo.email;
						poco_debug(Logger(),fmt::format("START({}): {} UI Client is starting WS connection.", Client->second->Id_, Client->second->UserName_));
						std::string S{"Welcome! Bienvenue! Bienvenidos!"};
						Client->second->WS_->sendFrame(S.c_str(), S.size());
                        Client->second->UserName_ = Client->second->UserInfo_.userinfo.email;
					} else {
						std::string S{"Invalid token. Closing connection."};
                        Client->second->WS_->sendFrame(S.c_str(), S.size());
                        return EndConnection(G, Client);
					}

				} else {
                    Poco::JSON::Parser P;
                    auto Obj =
                        P.parse(IncomingFrame.begin()).extract<Poco::JSON::Object::Ptr>();
                    std::string Answer;
                    bool CloseConnection=false;
                    if (Processor_ != nullptr) {
                        Processor_->Processor(Obj, Answer, CloseConnection);
                    }
                    if (!Answer.empty())
                        Client->second->WS_->sendFrame(Answer.c_str(), (int)Answer.size());
                    else {
                        Client->second->WS_->sendFrame("{}", 2);
                    }

                    if(CloseConnection) {
                        return EndConnection(G, Client);
                    }
				}
			} break;
			default: {
			}
			}
		} catch (...) {
            return EndConnection(G, Client);
		}
	}

	void UI_WebSocketClientServer::OnSocketShutdown([[maybe_unused]] const Poco::AutoPtr<Poco::Net::ShutdownNotification> &pNf) {
        ClientList::iterator Client;
        std::lock_guard     G(LocalMutex_);
        try {
            Client = FindWSClient(G, pNf->socket().impl()->sockfd());
            if (Client == end(Clients_))
                return;
            EndConnection(G, Client);
        } catch (...) {

        }
	}

} // namespace OpenWifi