//
//	License type: BSD 3-Clause License
//	License copy: https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/LICENSE
//
//	Created by Stephane Bourque on 2021-03-04.
//	Arilia Wireless Inc.
//


#ifndef UCENTRALFWS_RESTAPI_SERVER_H
#define UCENTRALFWS_RESTAPI_SERVER_H

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/NetException.h"

#include "framework/MicroService.h"

namespace OpenWifi {

    class RESTAPI_server : public SubSystemServer {

    public:
        static RESTAPI_server *instance() {
            if (instance_ == nullptr) {
                instance_ = new RESTAPI_server;
            }
            return instance_;
        }
        int Start() override;
        void Stop() override;
        void reinitialize(Poco::Util::Application &self) override;

    private:
        static RESTAPI_server *instance_;
        std::vector<std::unique_ptr<Poco::Net::HTTPServer>>   RESTServers_;
        Poco::ThreadPool	    Pool_;
        RESTAPI_GenericServer   LogServer_;

        RESTAPI_server() noexcept:
            SubSystemServer("RESTAPIServer", "RESTAPIServer", "openwifi.restapi")
            {
            }
    };

    inline RESTAPI_server * RESTAPI_server() { return RESTAPI_server::instance(); };

    class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
        public:
            explicit RequestHandlerFactory( RESTAPI_GenericServer & Server ) :
                    Logger_(RESTAPI_server::instance()->Logger()),
                    Server_(Server){}

            Poco::Net::HTTPRequestHandler *createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
        private:
            Poco::Logger            & Logger_;
            RESTAPI_GenericServer   & Server_;
    };
}

#endif //UCENTRALFWS_RESTAPI_SERVER_H
