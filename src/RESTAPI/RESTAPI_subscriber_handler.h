//
// Created by stephane bourque on 2021-11-07.
//

#pragma once

#include "framework/MicroService.h"

namespace OpenWifi {
    class RESTAPI_subscriber_handler : public RESTAPIHandler {
    public:
        RESTAPI_subscriber_handler(const RESTAPIHandler::BindingMap &bindings, Poco::Logger &L, RESTAPI_GenericServer & Server,
                                   uint64_t TransactionId, bool Internal)
        : RESTAPIHandler(bindings, L,
                         std::vector<std::string>{
            Poco::Net::HTTPRequest::HTTP_GET,
            Poco::Net::HTTPRequest::HTTP_PUT,
            Poco::Net::HTTPRequest::HTTP_DELETE,
            Poco::Net::HTTPRequest::HTTP_OPTIONS},
            Server,
            TransactionId,
            Internal, true, false, RateLimit{.Interval=1000,.MaxCalls=10}, true){}

        static auto PathName() { return std::list<std::string>{"/api/v1/subscriber"}; };

        void DoGet() final;
        void DoPost() final {};
        void DoPut() final;
        void DoDelete() final;
    private:
    };
}
