FROM ubuntu:14.04
MAINTAINER Marco Gulino <marco@gulinux.net>
COPY rootdir/ /
RUN apt-key add /etc/apt/sources.list.d/qt55-ppa.key && rm -f /etc/apt/sources.list.d/qt55-ppa.key && apt-get update
RUN apt-get install -y build-essential curl cimg-dev qt55base libgraphicsmagick++1-dev libgraphicsmagick++1-dev curl libglu1-mesa-dev \
    libopencv-dev libgphoto2-dev libcfitsio3-dev
RUN curl -O http://no.archive.ubuntu.com/ubuntu/pool/universe/c/cimg/cimg-dev_1.6.4+dfsg-1build1_all.deb && \
  dpkg -i /cimg-dev_1.6.4+dfsg-1build1_all.deb && \
  rm -f /cimg-dev_1.6.4+dfsg-1build1_all.deb

RUN echo 'source /opt/qt55/bin/qt55-env.sh' >> "$HOME/.bashrc"
RUN rm /bin/sh && ln -s /bin/bash /bin/sh
RUN source /opt/qt55/bin/qt55-env.sh && \
    curl http://www.qcustomplot.com/release/1.3.2/QCustomPlot-source.tar.gz | tar xz && \
    cd qcustomplot-source/ && cp *.h /usr/include && \
    curl http://www.qcustomplot.com/release/1.3.2/QCustomPlot-sharedlib.tar.gz | tar xz && \
    cd qcustomplot-sharedlib/sharedlib-compilation/ && \
    qmake && make && cp lib*.so* /usr/lib && cd /

RUN curl -L https://cmake.org/files/v3.5/cmake-3.5.2-Linux-x86_64.sh -O \
  && bash cmake-3.5.2-Linux-x86_64.sh --prefix=/usr --exclude-subdir && rm -f cmake-3.5.2-Linux-x86_64.sh

#RUN apt-get install -y libgraphicsmagick++1-dev
#RUN apt-get install -y libqcustomplot-dev
#RUN apt-get install -y libgphoto2-dev #libindi-dev libnova-dev

#RUN mkdir /GalaxyHunter /GalaxyHunter-dist /GalaxyHunter-install
#WORKDIR /GalaxyHunter-build
#CMD cmake /GalaxyHunter -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release && make -j4 && make package && cp -av GalaxyHunter* /GalaxyHunter-dist
