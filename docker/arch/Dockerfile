FROM archlinux/base

RUN pacman -Syuq --noconfirm base-devel git cmake glfw-x11 assimp

RUN echo "Defaults      lecture = never" > /etc/sudoers.d/privacy \
 && echo "%wheel ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/wheel \
 && useradd -m -G wheel -s /bin/bash circleci

USER circleci
WORKDIR /home/circleci

RUN git clone https://github.com/jdtaylor7/prometheus
