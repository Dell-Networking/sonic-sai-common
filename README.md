SONiC sai-common 
----------------------
This repo contains all SAI public API implementation that is used by the Dell SAI implementation.


Description
-----------

This repo has all public API implemenation for SAI.The NAS component utilizes this SAI API for NPU(network processor) related operations such as (port,l2,l3 related). 

Building
--------
Please see the instructions in the sonic-nas-manifest repo for more details on the common build tools.  [Sonic-nas-manifest](https://github.com/Azure/sonic-nas-manifest)

Development Dependencies:

 - sonic-logging
 - sonic-common-utils
 - sonic-nas-sai-api
 - sonic-sai-common-utils 

Dependent Packages:

 - libsonic-logging1 libsonic-logging-dev libsonic-common1 libsonic-common-dev sonic-sai-api-dev  libsonic-sai-common-utils1 libsonic-sai-common-utils-dev


BUILD CMD: sonic_build --dpkg libsonic-logging1 libsonic-logging-dev libsonic-common1 libsonic-common-dev sonic-sai-api-dev  libsonic-sai-common-utils1 libsonic-sai-common-utils-dev -- clean binary

(c) Dell 2016
