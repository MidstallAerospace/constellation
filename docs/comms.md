# Communications

Constellation uses three different endpoints for communication. The first one is on the vehicle and is called the "vehicle".
Next is "relay", which is on the ground. The next one is called "control", this has to obviously be on the ground. To simplify
communications, the payload is the same format throughout all nodes.

## Vehicle

This endpoint is the flight computer itself, this device will communicate to relay. The method used to communicate
between vehicle and relay may be done any way. This endpoint will receive all initialization data.

## Relay

This endpoint is responsible for communicating between the rocket and ground control. It hosts a TCP/IP server that
"control" connects to. There may be many or just one ground controller but there only is one relay. Some initialization
data may be sent from this.

## Control

This endpoint allows for humans to interact, monitor, and control the vehicle itself. Most or all initialization data
will be sent from this endpoint.
