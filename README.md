### Introduction

The purpose of this API client is to demonstrate how to interact with the Minesense Grade API.

More information on the Minesense Grade API can be requested from MineSense.

### Requirements

The following dependencies are needed for both the client and the server:

* CMAKE 3.4.1+
* A c++ compiler that supports C++14 or above
* [*IXWebSocket*](https://github.com/machinezone/IXWebSocket)
* [*json*](https://github.com/nlohmann/json)

To run the unit tests, gtests required:

* [*gtest*](https://github.com/google/googletest)

This sample API client has been tested on an Ubuntu system compiling with g++.

### Server Usage and Behaviour

`mtl-sample-api-server` contains a simplified WebSocket server. This application can be used to give the sample client something to run against before trying to connect to a ShovelSense system.

Details on building the sample server can be found in the "Building" section further down, but once built, running the server is as simple as running the executable:

`./server`

And accepts the following command-line arguments:

```
-h: help
-i, --ip: ip address server connects to
-p, --port: port server connects to
-v, --passive: server simulates sending live grade data as it is generated. Argument specifies grade frequency (in seconds)
```

By default, this process creates a server that runs on localhost and accepts connections through port 80. The sample client is configured to connect to this by default if it runs as a separate process on the same machine.

The server sends two types of messages: the Heartbeat message and the GradeResponse message. Both of these have the same format as the equivalent messages sent by a ShovelSense system.

The Heartbeat message is sent every 5 seconds. The GradeResponse message is sent in response to a well-formed GradeRequest message. The server will send back the following data:

```
{
    "batchStart": <equal to the midpoint between the startTime and endTime passed in through the gradeRequest>,
    "grades": [
        {
            "element": <the element(s) from the estimatedGrade(s) passed in through the gradeRequest>,
            "value": <a randomized grade value between 0.0 and 1.0>,
            "confidenceScore": 1.0
        }
    ],
    "haulingUnitID": <the haulingUnitId passed in through the gradeRequest>,
    "requestID": <the requestId passed in through the gradeRequest>,
    "systemID": "testServer",
    "type": "gradeResponse"
}
```

### Client Usage

`mtl-sample-api-client` contains a simplified WebSocket client. This application connects to the sample server, but can also be used to connect to a ShovelSense system running the MineSense Grade API.

Once built, the client can be run with default settings by simply running the executable:

`./client`

It supports the following command line options:

```
-h: help
-i, --ip: ip address client connects to
-p, --port: port client connects to
-r: Send request messages
-s, --start: start time to set for the grade request
-e, --end: end time to set for the grade request
-d, --id: message id to send
-u, --haulingunit: identifier for the truck, belt, etc that's sending this request
-t, --interval: frequency at which we set grade request
```

When successfully connected to the websocket, the client will receive a `heartbeat` message every X seconds.
`gradeResponse` messages will automatically be sent to the client when the publisher is on continious publishing mode.

Grades can also be requested by sending `gradeRequest` messages. Valid `gradeRequest` messages will return a `gradeResponse`
messages. Sending an invalid `gradeRequest` will return a `errorMessage`.

Any message received will be written to `output.txt`.

GradeRequests are JSON objects sent as JSON strings. A valid grade request has been
created in `main.cpp` and can be modified.


Detailed information regarding message fields and their data formats can be found in the Minesense Grade API document.


Example of valid grade request:
```
{
    "type": "gradeRequest",
    "version": "2.0"
    "requestID": "1",
    "startTime": "2019-03-25T22:15:00.000Z",
    "endTime": "2019-03-25T22:16:00.000Z",
    "haulingUnitID": "truck1",
    "position": "40° 26′ 46″ N 79° 58′ 56″ W",
    "estimatedGrade": [
        {
            "element": "Fe",
            "value": "1.35"
        }
    ]
}
```

Example of grade response:

```
{
    "batchEnd": "2019-05-25T22:16:00.000Z",
    "batchStart": "2019-05-25T22:15:00.000Z",
    "grades": [
        {
            "element": "Fe",
            "value": 3.3,
            "confidenceScore": 0.9
        },
        {
            "element": "Mo",
            "value": 2.2,
            "confidenceScore": 0.9
        }
    ],
    "haulingUnitID": "truck1",
    "requestID": "1",
    "systemID": "testServer",
    "type": "gradeResponse",
    "version": "2.0"
}

```

Example of heartbeat:
```
{
    "errorCode": 1,
    "systemID": "testServer",
    "type": "heartbeat",
    "version": "2.0"
}
```

### Building
Before building the client, we first need to build the IXWebSocket library. Under `SampleClient/IXWebSocket`,

```
mkdir build
cd build
cmake ../
make
```


To build the client, under `SampleClient/mtl-sample-api-client`,

```
mkdir build
cd build
cmake ../
make
```

Then to start the application
```
./client
```

Add `-r` to send grade requests, otherwise the client will only listen for heartbeat and passive
grade response messages
```
./client -r
```


The steps for building the sample server are essentially the same. Under `SampleClient/mtl-sample-api-server`,

```
mkdir build
cd build
cmake ../
make
```

Then to start the application
```
sudo ./server
```


To run the unit test, under `SampleClient/gtest`
```
mkdir test
cd test
cmake ../
make
```

Then to start the client unit tests
```
./client_unittest
```
