sonic-sai-common 
----------------------
This repo contains all switch abstraction interface (SAI) public application programming interface (API) implementation used by the Dell SAI implementation. All public API implementation is included in this repo for SAI. The network adaptation service (NAS) component utilizes this SAI API for network processor unit (NPU)-related operations such as port, l2, and l3 related. 

Build
--------
See [sonic-nas-manifest](https://github.com/Azure/sonic-nas-manifest) for more information on common build tools.

### Development dependencies
* `sonic-logging`
* `sonic-common-utils`
* `sonic-nas-sai-api`
* `sonic-sai-common-utils` 

### Dependent packages
* `libsonic-logging1` 
* `libsonic-logging-dev`
* `libsonic-common1`
* `libsonic-common-dev` 
* `sonic-sai-api-dev` 
* `libsonic-sai-common-utils1`
* `libsonic-sai-common-utils-dev`

BUILD CMD: sonic_build --dpkg libsonic-logging1 libsonic-logging-dev libsonic-common1 libsonic-common-dev sonic-sai-api-dev  libsonic-sai-common-utils1 libsonic-sai-common-utils-dev -- clean binary

(c) Dell 2016
