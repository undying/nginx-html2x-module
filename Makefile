
DOCKER_NAME=html2x

docker_run: docker_build docker_stop docker_clean
	docker run \
		--rm -it \
		--name $(DOCKER_NAME) \
		--net host \
		--privileged \
		$(DOCKER_NAME)

docker_stop:
	-docker stop $(DOCKER_NAME)

docker_clean:
	-docker rm $(DOCKER_NAME)

docker_build:
	docker build -t $(DOCKER_NAME) .

docker_exec:
	docker exec -it $(DOCKER_NAME) bash

request:
	rm -f /tmp/test.pdf
	curl \
		-o /tmp/test.pdf \
		-vd "@html/nginx news.htm" \
		-H 'Content-Type: text/html' \
		"127.0.0.1/html2pdf?dpi=150"
	xdg-open /tmp/test.pdf

docker_logs:
	docker exec -it $(DOCKER_NAME) tail -F /var/log/nginx/error.log

gdb:
	docker exec -it $(DOCKER_NAME) bash -c 'gdb --pid $$(pgrep -f worker)'

gdb_bt:
	docker exec -it $(DOCKER_NAME) bash -c 'gdb --pid $$(pgrep -f worker) -ex "thread apply all bt"'

html_to_pdf_test: src/html_to_pdf_test.c
	gcc \
    -Wall --std=c11 -pedantic -ggdb -g \
    src/html_to_pdf_test.c \
    -o html_to_pdf_test \
    -lwkhtmltox \
    -Wl,-rpath,/usr/local/lib

