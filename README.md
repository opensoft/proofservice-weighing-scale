# proofservice-weighing-scale
Proxy service for working with weighing scales (only HID-compatible for now)

How-to build
------------
#### Prereqs
 * libusb-1.0 (libusb-1.0-0-dev package in debian-based distributives)
 * Usually scales device will be created with root-only access. To fix it add file `/etc/udev/rules.d/50-ultegra-scale.rules` (name can be different) with next content (change vendor and product ids to relevant ones):
 ```
 SUBSYSTEMS=="usb", ATTRS{idVendor}=="0b67", ATTRS{idProduct}=="555e", GROUP="dialout", MODE="0666"
 ```

#### Build process
$PROOF_PATH is compulsory for both build and run-time. It should point to directory where Proof binaries and includes (i.e. product of Proof building) are placed.

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
