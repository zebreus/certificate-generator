FROM archlinux/base

MAINTAINER Lennart E.

#update db and improve mirrorlist
RUN pacman -Sy && pacman -S --noconfirm reflector && reflector --latest 200 --sort rate --save /etc/pacman.d/mirrorlist && pacman -Sy

#install build tools
RUN pacman -S --noconfirm base-devel git sudo

#add build user
RUN useradd builduser -m && passwd -d builduser && printf 'builduser ALL=(ALL) ALL\n' | tee -a /etc/sudoers

#get latex and thrift
RUN pacman -S --noconfirm texlive-core thrift

#build nlohmann/json from aur
RUN sudo -u builduser bash -c 'cd ~ && git clone https://aur.archlinux.org/nlohmann-json-bin.git nlohmann-json-bin && cd nlohmann-json-bin && makepkg -si --noconfirm'

RUN pacman -S --noconfirm boost

#install certificate generator
COPY ./ /certgen/
RUN mkdir -p /certgen/working/
RUN mkdir -p /certgen/output/
RUN make -C /certgen/ thrift && make -C /certgen/ certificate-generator-server

EXPOSE 9090

CMD /certgen/bin/certificate-generator-server
