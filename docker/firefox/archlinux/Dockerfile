FROM archlinux/base

RUN pacman -Syuq --noconfirm base-devel firefox

# Clear pacman cache.
RUN pacman -Scc

RUN echo "Defaults      lecture = never" > /etc/sudoers.d/privacy && \
    echo "%wheel ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/wheel && \
    useradd -m -G wheel -s /bin/bash circleci

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

CMD /usr/bin/firefox
