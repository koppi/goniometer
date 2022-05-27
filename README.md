![Overview](goniometer.png)

### Build, install and run

```bash
git clone https://github.com/koppi/goniometer
cd goniometer
sudo apt -y install libsdl2-dev libsdl2-image-dev portaudio19-dev libpulse-dev libsndio-dev
make
sudo make install
goniometer
```

Or build and install as a Debian/Ubuntu package:
```bash
git clone https://github.com/koppi/goniometer
cd goniometer
sudo DEBIAN_FRONTEND=noninteractive apt -qq -y install devscripts equivs
mk-build-deps -i -s sudo -t "apt --yes --no-install-recommends"
dpkg-buildpackage -b -rfakeroot -us -uc
sudo dpkg -i ../goniometer*.deb
sudo apt -f install
```
