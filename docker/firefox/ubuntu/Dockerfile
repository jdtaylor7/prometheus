FROM ubuntu:14.04

# Configure tzinfo.
ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York

RUN apt-get update --yes && apt-get upgrade --yes

RUN apt-get update && apt-get install -y firefox

# Clear apt-get cache.
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Replace 1000 with your user / group id
RUN export uid=1000 gid=1000 && \
    mkdir -p /home/circleci && \
    echo "circleci:x:${uid}:${gid}:Circleci,,,:/home/circleci:/bin/bash" >> /etc/passwd && \
    echo "circleci:x:${uid}:" >> /etc/group && \
    echo "circleci ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/circleci && \
    chmod 0440 /etc/sudoers.d/circleci && \
    chown ${uid}:${gid} -R /home/circleci

USER circleci
ENV HOME /home/circleci
WORKDIR /home/circleci

# CMD /usr/bin/firefox
