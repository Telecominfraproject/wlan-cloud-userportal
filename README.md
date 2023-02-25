<p align="center">
    <img src="images/project/logo.svg" width="200"/>
</p>

# OpenWifi User Self-Care portal (OWSUB)

## What is it?
The OWSUB is a service for the TIP OpenWiFi CloudSDK (OWSDK).
OWSUB provides a subscriber management service to the Subscriber App and interacts with the rest of the Cloud SDK. OWSUB, 
like all other OWSDK microservices, is defined using an OpenAPI definition and uses the ucentral communication 
protocol to interact with Access Points. To use the OWSUB, you either need to [build it](#building) or use the 
[Docker version](#docker).

## Building
In order to build the OWSUB, you will need to install its dependencies, which includes the following:
- cmake
- boost
- POCO 1.10.1 or later
- a C++17 compiler
- openssl
- libpq-dev (PortgreSQL development libraries)
- mysql-client (MySQL client)
- librdkafka
- cppkafka

The build is done in 2 parts. The first part is to build a local copy of the framework tailored to your environment. This
framework is called [Poco](https://github.com/pocoproject/poco). The version used in this project has a couple of fixes
from the master copy needed for cmake. Please use the version of this [Poco fix](https://github.com/AriliaWireless/poco). Building
Poco may take several minutes depending on the platform you are building on.

### Ubuntu
These instructions have proven to work on Ubuntu 20.4.
```bash
sudo apt install git cmake g++ libssl-dev libmariadb-dev 
sudo apt install libpq-dev libaprutil1-dev apache2-dev libboost-all-dev
sudo apt install librdkafka-dev default-libmysqlclient-dev
sudo apt install nlohmann-json-dev

cd ~
git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/AriliaWireless/cppkafka --branch tip-v1
cd cppkafka
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/AriliaWireless/valijson --branch tip-v1
cd valijson
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

git clone https://github.com/fmtlib/fmt --branch 9.0.0 /fmtlib
cd fmtlib
mkdir cmake-build
cd cmake-build
cmake ..
make
make install

cd ~
git clone https://github.com/Telecominfraproject/wlan-cloud-userportal
cd wlan-cloud-userportal
mkdir cmake-build
cd cmake-build
cmake ..
make -j 8
```

### Fedora
The following instructions have proven to work on Fedora 33
```bash
sudo yum install cmake g++ openssl-devel mysql-devel mysql apr-util-devel boost boost-devel
sudo yum install yaml-cpp-devel lua-devel 
sudo dnf install postgresql.x86_64 librdkafka-devel
sudo dnf install postgresql-devel json-devel

git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

git clone https://github.com/AriliaWireless/cppkafka --branch tip-v1
cd cppkafka
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/AriliaWireless/valijson --branch tip-v1
cd valijson
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/Telecominfraproject/wlan-cloud-userportal
cd wlan-cloud-userportal
mkdir cmake-build
cd cmake-build
cmake ..
make
```

### macOS Build
The following instructions have proven to work on macOS Big Sur. You need to install [Homebrew](https://brew.sh/). You must also have installed [XCode for OS X](https://www.freecodecamp.org/news/how-to-download-and-install-xcode/).
```bash
brew install openssl \
	cmake \
	libpq \
	mysql-client \
	apr \
	apr-util \
	boost \
	yaml-cpp \
	postgresql \
	librdkafka \
	nlohmann-json \
	fmt

git clone https://github.com/AriliaWireless/poco --branch poco-tip-v1
pushd poco
mkdir cmake-build
push cmake-build
cmake -DOPENSSL_ROOT_DIR=</path/to/openssl> -DENABLE_NETSSL=1 -DENABLE_JWT=1 -DENABLE_CRYPTO=1 ..
cmake --build . --config Release
sudo cmake --build . --target install
popd
popd

git clone https://github.com/AriliaWireless/cppkafka --branch tip-v1
pushd cppkafka
mkdir cmake-build
pushd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install
popd
popd

git clone https://github.com/AriliaWireless/valijson --branch tip-v1
cd valijson
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install
popd
popd

git clone https://github.com/Telecominfraproject/wlan-cloud-userportal
pushd wlan-cloud-userportal
mkdir cmake-build
pushd cmake-build
cmake ..
make -j
popd
popd
```

### Raspberry
The build on a rPI takes a while. You can shorten that build time and requirements by disabling all the larger database
support. You can build with only SQLite support by not installing the packages for PostgreSQL, and MySQL by
adding -DSMALL_BUILD=1 on the cmake build line.

```bash
sudo apt install git cmake g++ libssl-dev libaprutil1-dev apache2-dev libboost-all-dev libyaml-cpp-dev
git clone https://github.com/stephb9959/poco
cd poco
mkdir cmake-build
cd cmake-build
cmake ..
cmake --build . --config Release
sudo cmake --build . --target install

cd ~
git clone https://github.com/Telecominfraproject/wlan-cloud-userportal
cd wlan-cloud-userportal
mkdir cmake-build
cd cmake-build
cmake -DSMALL_BUILD=1 ..
make
```

### OWSUB Service Configuration
The configuration is kept in a file called `owsub.properties`. To understand the content of this file,
please look [here](https://github.com/Telecominfraproject/wlan-cloud-userportal/blob/main/CONFIGURATION.md)

## OpenAPI
You may get static page with OpenAPI docs generated from the definition on [GitHub Page](https://telecominfraproject.github.io/wlan-cloud-userportal/).

Also, you may use [Swagger UI](https://petstore.swagger.io/#/) with OpenAPI definition file raw link (i.e. [latest version file](https://raw.githubusercontent.com/Telecominfraproject/wlan-cloud-userportal/main/openapi/owanalytics.yaml)) to get interactive docs page.

## Firewall Considerations
| Port  | Description                                 | Configurable |
|:------|:--------------------------------------------|:------------:|
| 16006 | Default port for REST API Access to the OWSUB |     yes      |

## Kafka topics
Toe read more about Kafka, follow the [document](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/main/KAFKA.md)

## Contributions
We need more contributors. Should you wish to contribute,
please follow the [contributions](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/CONTRIBUTING.md) document.

## Pull Requests
Please create a branch with the Jira addressing the issue you are fixing or the feature you are implementing.
Create a pull-request from the branch into master.

## Additional OWSDK Microservices
Here is a list of additional OWSDK microservices
| Name | Description | Link | OpenAPI |
| :--- | :--- | :---: | :---: |
| OWSEC | Security Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralsec/blob/main/openpapi/owsec.yaml) |
| OWGW | Controller Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralgw/blob/master/openapi/owgw.yaml) |
| OWFMS | Firmware Management Service | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms) | [here](https://github.com/Telecominfraproject/wlan-cloud-ucentralfms/blob/main/openapi/owfms.yaml) |
| OWPROV | Provisioning Service | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov) | [here](https://github.com/Telecominfraproject/wlan-cloud-owprov/blob/main/openapi/owprov.yaml) |
| OWANALYTICS | Analytics Service | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics) | [here](https://github.com/Telecominfraproject/wlan-cloud-analytics/blob/main/openapi/owanalytics.yaml) |
| OWSUB | Subscriber Service | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal) | [here](https://github.com/Telecominfraproject/wlan-cloud-userportal/blob/main/openapi/userportal.yaml) |
