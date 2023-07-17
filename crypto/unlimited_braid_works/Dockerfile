FROM sagemath/sagemath:9.7

ENV DEBIAN_FRONTEND noninteractive

USER root
RUN apt-get update && apt-get install -yqq socat python3-pip
RUN pip install pycryptodome

USER sage
RUN sage -pip install pycryptodome
ADD challenge/server.sage server.sage

CMD socat TCP-L:9999,fork,reuseaddr EXEC:"sage server.sage"
