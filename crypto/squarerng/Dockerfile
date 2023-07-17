FROM python:3.11-slim

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -yqq socat
RUN pip install pycryptodome

WORKDIR /app
ADD distfiles/server.py .
CMD socat TCP-L:9999,fork,reuseaddr EXEC:"python server.py"
