# Predefined positions of the arm
# HOME, ZERO and COBRA are predefined pose. If you want to use them, set the target = NAMED_POSE and predefined_pose = HOME for ex 

string                      NAMED_POSE = "named_pose"
string                      HOME = "home"
string                      ZERO = "zero"
string                      COBRA = "cobra"
string                      ABOVE_GROUND = "above_ground"


# Tasks to perform

## Switches
## These are switches tasks. If you want to use them, no need to set the target data message

string                      BUTTON_1 = "switch_1"
string                      BUTTON_2 = "switch_2"
string                      BUTTON_3 = "switch_3"
string                      BUTTON_4 = "switch_4"
string                      BUTTON_5 = "switch_5"
string                      BUTTON_6 = "switch_6"
string                      BUTTON_7 = "switch_7"
string                      BUTTON_8 = "switch_8"
string                      BUTTON_9 = "switch_9"
string                      BUTTON_10 = "switch_10"
string                      BUTTON_11 = "switch_11"
string                      BUTTON_12 = "switch_12"
string                      BUTTON_13 = "switch_13"
string                      BUTTON_MAIN = "switch_main"

# socket steps: pickup, align aruco, align socket, insert socket, pullout
# metalbar steps: find, approach, pose estimation, pickup, align aruco, go left of electromagnet, place, release, go back
# ethernet steps: find, pickup, place in tool, close tool, grab back, open tool, align aruco, detect ethernet socket (?), align, insert, release, go back
# rock steps: find, go above, request detection, go to ground, grab, lift a bit, human verif, go above container, release
# brick steps: same as rock steps until grab step
# probe steps: same as rock until grab step, place into container

# useless or outdated or unclear custom messages to change:
    # Ethernet
    # Probe
    # Rock
    # RockArray
    # ArucoObject
    

string                      VOLTMETER_COMPLETE = "voltmeter"
string                      VOLTMETER_ALIGN = "voltmeter align"
string                      ETHERNET = "ethernet"
string                      ROCK = "rocks"
string                      PROBE = "probes"
string                      PROBE_STORE = "probe store
string                      DROP_SAMPLE = "drop sample"


## Tools manipulation
## If you want to use them, set target to TOOL_PICKUP for example, and tool to SHOVEL_TOOL

string                      TOOL_PICKUP = "tool pickup"
string                      TOOL_PLACEBACK = "tool place back"

### available tools

string                      SHOVEL_TOOL = "shovel"
string                      SOCKET_TOOL = "socket"
string                      BUTTON_TOOL = "buttons"





# ERROR_CODES

string                      OK = "ok"
string                      ALREADY_HAS_GOAL = "already has goal"
string                      UNKNOWN_GOAL = "unknown goal"
string                      UNDEFINED = "undefined"




# MESSAGE DATA
string                      target
string                      predefined_pose
string                      tool