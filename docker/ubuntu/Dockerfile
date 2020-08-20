FROM ubuntu:focal

# Configure tzinfo.
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York

RUN apt-get update --yes && apt-get upgrade --yes

RUN apt-get install -q --yes git g++ cmake libgl1-mesa-dev libglfw3 \
    libglfw3-dev libassimp-dev

RUN echo "Defaults      lecture = never" > /etc/sudoers \
 && useradd -m -G sudo -s /bin/bash circleci

USER circleci
WORKDIR /home/circleci

RUN git clone https://github.com/jdtaylor7/prometheus
