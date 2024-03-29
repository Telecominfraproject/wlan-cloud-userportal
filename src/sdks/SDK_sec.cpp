//
// Created by stephane bourque on 2022-01-11.
//

#include "SDK_sec.h"
#include "framework/MicroServiceNames.h"
#include "framework/OpenAPIRequests.h"

namespace OpenWifi::SDK::Sec {

	namespace User {
		bool Exists([[maybe_unused]] RESTAPIHandler *client, const Types::UUID_t &Id) {
			OpenAPIRequestGet Req(uSERVICE_SECURITY, "/api/v1/user/" + Id, {}, 5000);

			Poco::JSON::Object::Ptr Response;
			auto StatusCode = Req.Do(Response);
			if (StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
				return true;
			}
			return false;
		}

		bool Get([[maybe_unused]] RESTAPIHandler *client, const Types::UUID_t &Id,
				 SecurityObjects::UserInfo &UserInfo) {
			OpenAPIRequestGet Req(uSERVICE_SECURITY, "/api/v1/user/" + Id, {}, 5000);

			Poco::JSON::Object::Ptr Response;
			auto StatusCode = Req.Do(Response);
			if (StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
				return UserInfo.from_json(Response);
			}
			return false;
		}
	} // namespace User

	namespace Subscriber {
		bool Exists([[maybe_unused]] RESTAPIHandler *client, const Types::UUID_t &Id) {
			OpenAPIRequestGet Req(uSERVICE_SECURITY, "/api/v1/subuser/" + Id, {}, 5000);

			Poco::JSON::Object::Ptr Response;
			auto StatusCode = Req.Do(Response);
			if (StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
				return true;
			}
			return false;
		}

		bool Get([[maybe_unused]] RESTAPIHandler *client, const Types::UUID_t &Id,
				 SecurityObjects::UserInfo &UserInfo) {
			OpenAPIRequestGet Req(uSERVICE_SECURITY, "/api/v1/subuser/" + Id, {}, 5000);

			Poco::JSON::Object::Ptr Response;
			auto StatusCode = Req.Do(Response);
			if (StatusCode == Poco::Net::HTTPResponse::HTTP_OK) {
				return UserInfo.from_json(Response);
			}
			return false;
		}

		bool Delete(RESTAPIHandler *client, const Types::UUID_t &Id) {
			OpenAPIRequestDelete Req(uSERVICE_SECURITY, "/api/v1/subuser/" + Id, {}, 15000);

			auto StatusCode =
				Req.Do(client == nullptr ? "" : client->UserInfo_.webtoken.access_token_);
			if (StatusCode >= 200 && StatusCode <= 204) {
				return true;
			}
			return false;
		}

	} // namespace Subscriber

} // namespace OpenWifi::SDK::Sec
