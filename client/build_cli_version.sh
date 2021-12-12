docker run \
    --rm -i --net=none \
    -v "${PWD}:/app" \
    -w /app \
    ast:latest /app/in_docker_build_cli_version.sh 