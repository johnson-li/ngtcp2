#!/bin/bash

for row in $(cat index.csv); do
    examples/client 127.0.0.1 4433 -i -w $row -q
done
echo "evaluate the top websites done"
