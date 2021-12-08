docker run \
    --rm -i --net=none \
    -v "${PWD}:/app" \
    -w /app \
    ast:latest /app/in_docker_build.sh 