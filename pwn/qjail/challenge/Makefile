all:
	docker cp main.c pwn_231:/root/main.c
	docker exec -it pwn_231 gcc /root/main.c -o /tmp/vuln
	docker cp pwn_231:/tmp/vuln .
