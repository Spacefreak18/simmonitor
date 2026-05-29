#dnf install argtable-devel libconfig-devel libuv-devel libxdg-basedir-devel procps-ng-devel sdl2-compat-devel libarchive-devel ncurses-devel libmicrohttpd-devel hoel-devel
Summary: A dashboard and telemetry manager for racing sims
Name: simmonitor
Version: 0.0.5
Release: 1
License: GPL
Group: Applications/Sound
Source: https://github.com/simmonitor
URL: https://spacefreak18.github.io/simapi
Distribution: Fedora Linux
Vendor: spacefreak18
Packager: Paul Jones <paul@spacefreak18.xyz>
Requires: argtable libconfig libuv libxdg-basedir procps-ng sdl2-compat libarchive ncurses libmicrohttpd libhoel

%description
A dashboard and telemetry manager for Racing sims

%prep
rm -rf $RPM_BUILD_DIR/simmonitor
rm -rf $RPM_SOURCE_DIR/simmonitor
cd $RPM_SOURCE_DIR
git clone https://github.com/spacefreak18/simmonitor
cd simmonitor
git submodule update --init --recursive
cd ..
cp -r $RPM_SOURCE_DIR/simmonitor $RPM_BUILD_DIR/

%build
cd $RPM_BUILD_DIR/simmonitor
cmake -B build
cd build
make

%install
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/simmonitor/build/simmonitor $RPM_BUILD_ROOT/usr/bin/simmonitor

%files
/usr/bin/simmonitor
