FROM ubuntu:15.10
MAINTAINER Marco Gulino <marco@gulinux.net>

RUN apt-get update && apt-get install -y build-essential cmake qtbase5-dev pkg-config libopencv-dev cimg-dev libraw-dev libjpeg-dev libgphoto2-dev libcfitsio-dev libqcustomplot-dev
RUN apt-get install -y libgraphicsmagick++1-dev # TODO: remove

RUN mkdir /GalaxyHunter /GalaxyHunter-dist /GalaxyHunter-install
WORKDIR /GalaxyHunter-build
CMD cmake /GalaxyHunter -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DENABLE_INDI=Off && make -j4 && make package && cp -av GalaxyHunter* /GalaxyHunter-dist
