#!/bin/sh
set -e

if [ "$SELFSIGNED_CERTS" = 'true' ]; then
    update-ca-certificates
fi

if [[ "$TEMPLATE_CONFIG" = 'true' ]]; then
  RESTAPI_HOST_ROOTCA=${RESTAPI_HOST_ROOTCA:-"\$OWSUB_ROOT/certs/restapi-ca.pem"} \
  RESTAPI_HOST_PORT=${RESTAPI_HOST_PORT:-"16006"} \
  RESTAPI_HOST_CERT=${RESTAPI_HOST_CERT:-"\$OWSUB_ROOT/certs/restapi-cert.pem"} \
  RESTAPI_HOST_KEY=${RESTAPI_HOST_KEY:-"\$OWSUB_ROOT/certs/restapi-key.pem"} \
  RESTAPI_HOST_KEY_PASSWORD=${RESTAPI_HOST_KEY_PASSWORD:-"mypassword"} \
  INTERNAL_RESTAPI_HOST_ROOTCA=${INTERNAL_RESTAPI_HOST_ROOTCA:-"\$OWSUB_ROOT/certs/restapi-ca.pem"} \
  INTERNAL_RESTAPI_HOST_PORT=${INTERNAL_RESTAPI_HOST_PORT:-"17006"} \
  INTERNAL_RESTAPI_HOST_CERT=${INTERNAL_RESTAPI_HOST_CERT:-"\$OWSUB_ROOT/certs/restapi-cert.pem"} \
  INTERNAL_RESTAPI_HOST_KEY=${INTERNAL_RESTAPI_HOST_KEY:-"\$OWSUB_ROOT/certs/restapi-key.pem"} \
  INTERNAL_RESTAPI_HOST_KEY_PASSWORD=${INTERNAL_RESTAPI_HOST_KEY_PASSWORD:-"mypassword"} \
  SERVICE_KEY=${SERVICE_KEY:-"\$OWSUB_ROOT/certs/restapi-key.pem"} \
  SERVICE_KEY_PASSWORD=${SERVICE_KEY_PASSWORD:-"mypassword"} \
  SYSTEM_DATA=${SYSTEM_DATA:-"\$OWSUB_ROOT/data"} \
  SYSTEM_URI_PRIVATE=${SYSTEM_URI_PRIVATE:-"https://localhost:17006"} \
  SYSTEM_URI_PUBLIC=${SYSTEM_URI_PUBLIC:-"https://localhost:16006"} \
  SYSTEM_URI_UI=${SYSTEM_URI_UI:-"http://localhost"} \
  SECURITY_RESTAPI_DISABLE=${SECURITY_RESTAPI_DISABLE:-"false"} \
  KAFKA_ENABLE=${KAFKA_ENABLE:-"true"} \
  KAFKA_BROKERLIST=${KAFKA_BROKERLIST:-"localhost:9092"} \
  KAFKA_SSL_CA_LOCATION=${KAFKA_SSL_CA_LOCATION:-""} \
  KAFKA_SSL_CERTIFICATE_LOCATION=${KAFKA_SSL_CERTIFICATE_LOCATION:-""} \
  KAFKA_SSL_KEY_LOCATION=${KAFKA_SSL_KEY_LOCATION:-""} \
  KAFKA_SSL_KEY_PASSWORD=${KAFKA_SSL_KEY_PASSWORD:-""} \
  STORAGE_TYPE=${STORAGE_TYPE:-"sqlite"} \
  STORAGE_TYPE_POSTGRESQL_HOST=${STORAGE_TYPE_POSTGRESQL_HOST:-"localhost"} \
  STORAGE_TYPE_POSTGRESQL_USERNAME=${STORAGE_TYPE_POSTGRESQL_USERNAME:-"owsub"} \
  STORAGE_TYPE_POSTGRESQL_PASSWORD=${STORAGE_TYPE_POSTGRESQL_PASSWORD:-"owsub"} \
  STORAGE_TYPE_POSTGRESQL_DATABASE=${STORAGE_TYPE_POSTGRESQL_DATABASE:-"owsub"} \
  STORAGE_TYPE_POSTGRESQL_PORT=${STORAGE_TYPE_POSTGRESQL_PORT:-"5432"} \
  STORAGE_TYPE_MYSQL_HOST=${STORAGE_TYPE_MYSQL_HOST:-"localhost"} \
  STORAGE_TYPE_MYSQL_USERNAME=${STORAGE_TYPE_MYSQL_USERNAME:-"owsub"} \
  STORAGE_TYPE_MYSQL_PASSWORD=${STORAGE_TYPE_MYSQL_PASSWORD:-"owsub"} \
  STORAGE_TYPE_MYSQL_DATABASE=${STORAGE_TYPE_MYSQL_DATABASE:-"owsub"} \
  STORAGE_TYPE_MYSQL_PORT=${STORAGE_TYPE_MYSQL_PORT:-"3306"} \
  envsubst < /owsub.properties.tmpl > $OWSUB_CONFIG/owsub.properties
fi

if [ "$1" = '/openwifi/owsub' -a "$(id -u)" = '0' ]; then
    if [ "$RUN_CHOWN" = 'true' ]; then
      chown -R "$OWSUB_USER": "$OWSUB_ROOT" "$OWSUB_CONFIG"
    fi
    exec su-exec "$OWSUB_USER" "$@"
fi

exec "$@"
