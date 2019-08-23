FROM archlinux/base

MAINTAINER Lennart E.

RUN pacman -Sy && pacman -S --noconfirm texlive-core thrift

COPY ./ /certgen/

EXPOSE 9090

CMD /certgen/bin/certificate-generator-server
