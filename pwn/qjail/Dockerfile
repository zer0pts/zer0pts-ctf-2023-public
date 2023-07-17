FROM ubuntu:22.04

RUN apt-get -y update --fix-missing
RUN apt-get -y upgrade
RUN apt-get -y install socat python3 python3-pip
RUN groupadd -r ctf && useradd -r -g ctf ctf
RUN chmod 1733 /tmp /var/tmp /dev/shm
RUN pip3 install qiling

ADD distfiles /home/ctf

WORKDIR /home/ctf
RUN echo "zer0pts{Th1s_j4Il_f33Ls_m0R3_c0mF0rt4bL3_tH4n_r34L_3nv1r0nm3nt}" > flag.txt
RUN chmod 440 flag.txt
RUN chmod 550 sandbox.py

RUN chown -R root:ctf /home/ctf

USER ctf
CMD socat TCP-L:9999,reuseaddr,fork EXEC:"./sandbox.py ./bin/vuln"
