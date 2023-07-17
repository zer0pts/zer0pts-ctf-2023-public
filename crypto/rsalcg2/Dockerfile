FROM python:3.10-slim-buster

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -yqq socat
RUN pip install pycryptodome
ADD challenge/server.py server.py

ENV FLAG "zer0pts{i_h0pe_y0u_d1d_n0t_v10l3ntly_5o1v3_th1s_w1th_th3_c0mput4t10n4l_p0w3r}"
CMD socat TCP-L:9999,fork,reuseaddr EXEC:"python server.py"
