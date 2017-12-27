# proofservice-weighing-scale
Proxy service for working with weighing scales (only HID-compatible for now)

How-to build
------------
#### Prereqs
 * libusb-1.0 (libusb-1.0-0-dev package in debian-based distributives)

#### Build process
$PROOF_PATH is compulsory for both build and run-time. It should point to directory where Proof binaries and includes (i.e. artifacts of Proof compilation) are placed.

Set $QMAKEFEATURES var to $PROOF_PATH/features dir (needed only for build itself).

Build process:
```bash
mkdir build
cd build
qmake ../proofservice-weighing-scale.pro
make -jN
```
Where N is number of compile tasks that will run in parallel.

Also proof building scripts can be used to build Debian package. proof and proof-dev packages should be installed.

```bash
/opt/Opensoft/proof/tools/build-deb-package -f Manifest
```

Package will be placed to the project root.

How-to use
----------
Config changes are applied only at service restart. If service was installed using deb package then auto restarter should be enabled and simple kill -9 SERVICE_PID will work.

Usually scale device will be created with root-only access. To fix it add file `/etc/udev/rules.d/50-ultegra-scale.rules` (name can be different) with next content (change vendor and product ids to relevant ones):
```
SUBSYSTEMS=="usb", ATTRS{idVendor}=="0b67", ATTRS{idProduct}=="555e", GROUP="dialout", MODE="0666"
```

#### Config example
```ini
[General]
# HID vendor and product ids (can be retrieved from lsusb output)
vendor_id=0x0b67
product_id=0x555e

[server]
# server port
port=9000

[error_notifier]
# app_id which will be added to all emails and system/status or system/recent-errors endpoints
# doesn't do anything else, is used only to separate one instance from another
app_id=DotG-Shipping-Area-Weighing-Scale
# Email section describes credentials for email-based error notifications
# If not enabled nothing will be sent
email\enabled=false
email\from=
email\host=
email\password=
email\port=25
email\to=
# possible values are plain, ssl and starttls
email\type=ssl
email\username=

[updates]
# Should service try to update itself from time to time or not using apt-get(apt-get update && apt-get install)
auto_update=false
# files with sources that should be apt-get updated. If empty all sources will be updated
sources_list_file=
```
