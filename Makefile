
DOCKER_NAME=pdf

docker_run: docker_build docker_stop docker_clean
	docker run \
		--rm -it \
		--name $(DOCKER_NAME) \
		--net host \
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
	curl -vd '<html><body>Hello!</body></html>' 127.0.0.1
