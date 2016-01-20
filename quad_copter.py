"""
quad copter
"""

from dronekit import connect, VehicleMode, LocationGlobalRelative, LocationGlobal, Command
import time
import math
from pymavlink import mavutil

def quadCopter_create_handle(master_ip):
    """
    connect to vehicle device
    """
    print 'Connecting to vehicle on: %s' % master_ip
    vehicle = connect(master_ip, wait_ready=True)
    vehicle.groundspeed = 3
    return vehicle


def quadCopter_destory_handle(vehicle):
    """
    close vehicle device
    """
    print "Close vehicle object"
    vehicle.close()


def quadCopter_takeoff(vehicle, target_altitude_metres, callback2c_report_altitude=None):
    """
    take off copter to target_altitude_metres
    """
    print "Basic pre-arm checks"
    # Don't let the user try to arm until autopilot is ready
    while not vehicle.is_armable:
        print " Waiting for vehicle to init: mode.name (%s) != INITIALISING, fix_type(%s) > 1" % (vehicle.mode.name, vehicle.gps_0.fix_type)
        time.sleep(1)
        break

    print "Arming motors"
    # Copter should arm in GUIDED mode
    vehicle.mode    = VehicleMode("GUIDED")
    vehicle.armed   = True

    while not vehicle.armed:
        print " Waiting for arming..."
        time.sleep(1)

    print "Taking off!"
    vehicle.simple_takeoff(target_altitude_metres) # Take off to target altitude

    # Wait until the vehicle reaches a safe height before processing the goto (otherwise the command
    #  after Vehicle.simple_takeoff will execute immediately).
    while True:
        print "altitude:"
        if callback2c_report_altitude:
            callback2c_report_altitude(vehicle.location.global_relative_frame.alt)
        if vehicle.location.global_relative_frame.alt>=target_altitude_metres*0.95: #Trigger just below target alt.
            if callback2c_report_altitude:
                callback2c_report_altitude(vehicle.location.global_relative_frame.alt)
            print "*** Reached target altitude ***"
            break;
        time.sleep(1)


def get_location_with_metres(original_location, dNorth, dEast):
    """
    Returns a LocationGlobal object containing the latitude/longitude `dNorth` and `dEast` metres from the
    specified `original_location`. The returned LocationGlobal has the same `alt` value
    as `original_location`.
    """
    earth_radius=6378137.0 #Radius of "spherical" earth
    #Coordinate offsets in radians
    dLat = dNorth/earth_radius
    dLon = dEast/(earth_radius*math.cos(math.pi*original_location.lat/180))

    #New position in decimal degrees
    newlat = original_location.lat + (dLat * 180/math.pi)
    newlon = original_location.lon + (dLon * 180/math.pi)
    if type(original_location) is LocationGlobal:
        targetlocation = LocationGlobal(newlat, newlon, original_location.alt)
    elif type(original_location) is LocationGlobalRelative:
        targetlocation = LocationGlobalRelative(newlat, newlon, original_location.alt)
    else:
        raise Exception("Invalid Location object passed")

    return targetlocation;


def get_distance_metres(aLocation1, aLocation2):
    """
    Returns the ground distance in metres between two LocationGlobal objects.
    """
    dlat = aLocation2.lat - aLocation1.lat
    dlong = aLocation2.lon - aLocation1.lon
    return math.sqrt((dlat*dlat) + (dlong*dlong)) * 1.113195e5


def shift_position(vehicle, move_north_metres, move_east_metres, move_rise_metres):
    """
    shift path to avoid hit something
    """
    print "    shift_north %s, shift_east %s, shift_rise %s" % (move_north_metres, move_east_metres, move_rise_metres)

    vehicle.mode = VehicleMode("GUIDED")
    while vehicle.mode.name != "GUIDED":
        time.sleep(2)

    currentLocation = vehicle.location.global_relative_frame
    currentLocation.alt = currentLocation.alt + move_rise_metres
    targetLocation = get_location_with_metres(currentLocation, move_north_metres, move_east_metres)
    targetDistance = get_distance_metres(currentLocation, targetLocation)

    vehicle.simple_goto(targetLocation)

    while vehicle.mode.name == "GUIDED":
        remainingDistance = get_distance_metres(vehicle.location.global_relative_frame, targetLocation)
        currentLocation = vehicle.location.global_relative_frame

        reached_target = 0
        offset_lat = currentLocation.lat - targetLocation.lat
        offset_lon = currentLocation.lon - targetLocation.lon
        if offset_lat < 0:
            offset_lat = offset_lat * -1

        if offset_lon < 0:
            offset_lon = offset_lon * -1

        if offset_lat < 0.000005 and offset_lon < 0.000005:
            reached_target = 1
            print "  gps offset %s, %s" % (offset_lat, offset_lon)

        if reached_target: # or remainingDistance <= targetDistance*0.05:
            print "*** shift path ready ***"
            break;
        time.sleep(2)


def quadCopter_goto_relative_position(vehicle, north_metres, east_metres, rise_metres, callback2c_report_remaining_distance=None):
    """
    go to the position which is relative to current
    """
    currentLocation = vehicle.location.global_relative_frame
    currentLocation.alt = currentLocation.alt + rise_metres
    targetLocation = get_location_with_metres(currentLocation, north_metres, east_metres)
    targetDistance = get_distance_metres(currentLocation, targetLocation)

    print " target pos: %s" % targetLocation;
    vehicle.simple_goto(targetLocation)

    while vehicle.mode.name == "GUIDED": #Stop action if we are no longer in guided mode.
        remainingDistance = get_distance_metres(vehicle.location.global_relative_frame, targetLocation)
        currentLocation = vehicle.location.global_relative_frame
        print "Distance to target:"
        if callback2c_report_remaining_distance:
            move_north_metres, move_east_metres, move_rise_metres = callback2c_report_remaining_distance(remainingDistance, currentLocation.lat, currentLocation.lon, currentLocation.alt)
            if move_north_metres or move_east_metres or move_rise_metres:
                # hold current position
                vehicle.mode = VehicleMode("POSHOLD")
                while vehicle.mode.name != "POSHOLD":
                    time.sleep(2)

                shift_position(vehicle, move_north_metres, move_east_metres, move_rise_metres)

                # re-calculate path
                currentLocation = vehicle.location.global_relative_frame
                targetDistance = get_distance_metres(currentLocation, targetLocation)

                vehicle.simple_goto(targetLocation)
                continue

        reached_target = 0
        offset_lat = currentLocation.lat - targetLocation.lat
        offset_lon = currentLocation.lon - targetLocation.lon
        if offset_lat < 0:
            offset_lat = offset_lat * -1

        if offset_lon < 0:
            offset_lon = offset_lon * -1

        if offset_lat < 0.000005 and offset_lon < 0.000005:
            reached_target = 1
            print "  gps offset %s, %s" % (offset_lat, offset_lon)

        if reached_target: # or remainingDistance <= targetDistance*0.05: #Just below target, in case of undershoot.
            if callback2c_report_remaining_distance:
                move_north_metres, move_east_metres, move_rise_metres = callback2c_report_remaining_distance(remainingDistance, currentLocation.lat, currentLocation.lon, currentLocation.alt)
            print "*** Reached target ***"
            break;
        time.sleep(2)


def quadCopter_goto_gps_position(vehicle, latitude, longitude, altitude, callback2c_report_remaining_distance=None):
    """
    go to the position GPS coordinates
    """
    current_gps = vehicle.location.global_relative_frame
    target_gps = LocationGlobal(latitude, longitude, altitude)
    targetDistance = get_distance_metres(current_gps, target_gps)

    print " target GPS pos: %s" % target_gps;
    vehicle.simple_goto(target_gps)

    while vehicle.mode.name == "GUIDED": #Stop action if we are no longer in guided mode.
        remainingDistance = get_distance_metres(vehicle.location.global_relative_frame, target_gps)
        current_gps = vehicle.location.global_relative_frame
        print "Distance to target:"
        if callback2c_report_remaining_distance:
            move_north_metres, move_east_metres, move_rise_metres = callback2c_report_remaining_distance(remainingDistance, current_gps.lat, current_gps.lon, current_gps.alt)
            if move_north_metres or move_east_metres or move_rise_metres:
                # hold current position
                vehicle.mode = VehicleMode("POSHOLD")
                while vehicle.mode.name != "POSHOLD":
                    time.sleep(2)

                shift_position(vehicle, move_north_metres, move_east_metres, move_rise_metres)

                # re-calculate path
                current_gps = vehicle.location.global_relative_frame
                targetDistance = get_distance_metres(current_gps, target_gps)

                vehicle.simple_goto(target_gps)
                continue

        reached_target = 0
        offset_lat = current_gps.lat - target_gps.lat
        offset_lon = current_gps.lon - target_gps.lon
        if offset_lat < 0:
            offset_lat = offset_lat * -1

        if offset_lon < 0:
            offset_lon = offset_lon * -1

        if offset_lat < 0.000005 and offset_lon < 0.000005:
            reached_target = 1
            print "  gps offset %s, %s" % (offset_lat, offset_lon)

        if reached_target: # or remainingDistance <= targetDistance*0.05: #Just below target, in case of undershoot.
            if callback2c_report_remaining_distance:
                move_north_metres, move_east_metres, move_rise_metres = callback2c_report_remaining_distance(remainingDistance, current_gps.lat, current_gps.lon, current_gps.alt)
            print "*** Reached target GPS ***"
            break;
        time.sleep(2)


def quadCopter_get_curr_position(vehicle):
    """
    get current position
    """
    currLocation = vehicle.location.global_frame
    print "local %s" % (currLocation)
    return currLocation.lat, currLocation.lon, currLocation.alt



def quadCopter_set_flight_mode(vehicle, target_mode):
    """
    set flight mode
    """
    # print "set mode %s" % target_mode
    vehicle.mode = VehicleMode(target_mode)
    while vehicle.mode != target_mode:
        # print "check mode: %s" % vehicle.mode
        time.sleep(2)

    print "check mode: %s" % vehicle.mode


def quadCopter_get_flight_mode(vehicle):
    """
    get flight mode
    """
    # print "get mode: %s" % vehicle.mode.name
    return vehicle.mode.name


def quadCopter_set_yaw(vehicle, heading, relative=False):
    """
    Send MAV_CMD_CONDITION_YAW message to point vehicle at a specified heading (in degrees).
    """
    currentLocation = vehicle.location.global_relative_frame
    targetLocation = get_location_with_metres(currentLocation, 0, 0)

    print " target pos: %s" % targetLocation;
    vehicle.simple_goto(targetLocation)

    if relative:
        is_relative = 1 #yaw relative to direction of travel
    else:
        is_relative = 0 #yaw is an absolute angle

    print "yaw: %s, relative: %s" % (heading, is_relative)
    # create the CONDITION_YAW command using command_long_encode()
    msg = vehicle.message_factory.command_long_encode(
        0, 0,    # target system, target component
        mavutil.mavlink.MAV_CMD_CONDITION_YAW, #command
        0, #confirmation
        heading,    # param 1, yaw in degrees
        0,          # param 2, yaw speed deg/s
        1,          # param 3, direction -1 ccw, 1 cw
        is_relative, # param 4, relative offset 1, absolute angle 0
        0, 0, 0)    # param 5 ~ 7 not used
    # send command to vehicle
    vehicle.send_mavlink(msg)
    time.sleep(2)


def quadCopter_clear_mission(vehicle):
    """
    clear missions in the vehicle.
    """
    cmds = vehicle.commands
    cmds.clear()

    # MAV_CMD_NAV_TAKEOFF will be ignored if the vehicle is already in the air.
    cmds.add(Command(0, 0, 0, mavutil.mavlink.MAV_FRAME_GLOBAL_RELATIVE_ALT, mavutil.mavlink.MAV_CMD_NAV_TAKEOFF,
                     0, 0, 0, 0, 0, 0, 0, 0, 10))
    cmds.upload()
    time.sleep(5)


def quadCopter_add_relative_waypoint(vehicle, north_metres, east_metres, rise_metres):
    """
    add a waypoint which is relative current position
    """
    cmds = vehicle.commands
    currentLocation = vehicle.location.global_relative_frame
    currentLocation.alt = rise_metres
    targetLocation = get_location_with_metres(currentLocation, north_metres, east_metres)

    print " Add new wp %s" % targetLocation
    cmds.add(Command(0, 0, 0, mavutil.mavlink.MAV_FRAME_GLOBAL_RELATIVE_ALT, mavutil.mavlink.MAV_CMD_NAV_WAYPOINT,
                     0, 0, 0, 0, 0, 0, targetLocation.lat, targetLocation.lon, targetLocation.alt))


def quadCopter_upload_mission(vehicle):
    """
    upload missions to the vehicle.
    """
    cmds = vehicle.commands
    cmds.upload()
    time.sleep(5)


def quadCopter_download_curr_mission(vehicle, callback2c_report_waypoint=None):
    """
    Download the current mission from the vehicle.
    """
    cmds = vehicle.commands
    wp_list = []
    cmds.download()
    cmds.wait_ready()
    for cmd in cmds:
        # print "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (cmd.seq, cmd.current, cmd.frame, cmd.command,
        #                                                             cmd.param1, cmd.param2, cmd.param3, cmd.param4,
        #                                                             cmd.x, cmd.y, cmd.z, cmd.autocontinue)

        cmd_items = []

        cmd_items.append(cmd.seq)
        cmd_items.append(cmd.current)
        cmd_items.append(cmd.frame)
        cmd_items.append(cmd.command)
        cmd_items.append(cmd.x)
        cmd_items.append(cmd.y)
        cmd_items.append(cmd.z)
        cmd_items.append(cmd.autocontinue)
        if callback2c_report_waypoint:
            callback2c_report_waypoint(cmd_items)
        wp_list.append(cmd_items)

    return wp_list


def qaudcopter_launch_monitor_mission(vehicle, callback2c_report_mission_stat=None):
    """
    Download the current mission from the vehicle.
    """
    while True:
        nextwaypoint = vehicle.commands.next
        endwaypoint  = vehicle.commands.count
        if nextwaypoint == 0 or nextwaypoint == endwaypoint:
            print "no waypoint~~"
            break

        missionitem = vehicle.commands[nextwaypoint-1] #commands are zero indexed
        lat = missionitem.x
        lon = missionitem.y
        alt = missionitem.z
        targetWaypointLocation = LocationGlobalRelative(lat, lon, alt)
        distance2point = get_distance_metres(vehicle.location.global_frame, targetWaypointLocation)

        if callback2c_report_mission_stat:
            currentLocation = vehicle.location.global_relative_frame
            move_north_metres, move_east_metres, move_rise_metres = callback2c_report_mission_stat(distance2point, nextwaypoint,
                                                                        currentLocation.lat, currentLocation.lon, currentLocation.alt)
            if move_north_metres or move_east_metres or move_rise_metres:
                # hold current position
                vehicle.mode = VehicleMode("POSHOLD")
                while vehicle.mode.name != "POSHOLD":
                    time.sleep(2)

                shift_position(vehicle, move_north_metres, move_east_metres, move_rise_metres)

                # reset current waypoint index in this mission
                vehicle.commands.next = nextwaypoint

                # auto-trigger this mission
                vehicle.mode = VehicleMode("AUTO")
                while vehicle.mode.name != "AUTO":
                    time.sleep(1)


        print 'Distance to waypoint (%s/%s): %s' % (nextwaypoint, vehicle.commands.count, distance2point)

        time.sleep(1)


def print_mission_file(aFileName):
    """
    Print a mission file to demonstrate "round trip"
    """
    print "\nMission file: %s" % aFileName
    with open(aFileName) as f:
        for line in f:
            print ' %s' % line.strip()


def quadCopter_import_mission(vehicle, file_path):
    """
    import a mission from a file.
    """
    print "\nReading mission from file: %s" % file_path
    cmds = vehicle.commands
    missionlist = []
    with open(file_path) as f:
        for i, line in enumerate(f):
            if i == 0:
                if not line.startswith('QGC WPL 110'):
                    raise Exception('File is not supported WP version')
            else:
                linearray=line.split('\t')
                ln_index=int(linearray[0])
                ln_currentwp=int(linearray[1])
                ln_frame=int(linearray[2])
                ln_command=int(linearray[3])
                ln_param1=float(linearray[4])
                ln_param2=float(linearray[5])
                ln_param3=float(linearray[6])
                ln_param4=float(linearray[7])
                ln_param5=float(linearray[8])
                ln_param6=float(linearray[9])
                ln_param7=float(linearray[10])
                ln_autocontinue=int(linearray[11].strip())
                cmd = Command(0, 0, 0, ln_frame, ln_command, ln_currentwp, ln_autocontinue,
                              ln_param1, ln_param2, ln_param3, ln_param4, ln_param5, ln_param6, ln_param7)
                missionlist.append(cmd)

    print "\nUpload mission "

    # Clear existing mission from vehicle
    cmds.clear()

    # Add new mission to vehicle
    for command in missionlist:
        cmds.add(command)

    cmds.upload()
    time.sleep(5)


def quadCopter_export_mission(vehicle, file_path):
    """
    export a mission to a file.
    """
    print " Download mission from vehicle"
    missionlist = []
    cmds = vehicle.commands
    cmds.download()
    cmds.wait_ready()
    for cmd in cmds:
        missionlist.append(cmd)

    # Add file-format information
    output='QGC WPL 110\n'

    # Add home location as 0th waypoint
    home = vehicle.home_location
    output += "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (0, 1, mavutil.mavlink.MAV_FRAME_GLOBAL_RELATIVE_ALT, mavutil.mavlink.MAV_CMD_NAV_TAKEOFF,
                                                                    0, 0, 0, 0, home.lat, home.lon, home.alt, 1)

    # Add commands
    for cmd in missionlist:
        commandline="%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (cmd.seq, cmd.current, cmd.frame, cmd.command,
                                                                          cmd.param1, cmd.param2, cmd.param3, cmd.param4,
                                                                          cmd.x, cmd.y, cmd.z, cmd.autocontinue)
        output += commandline

    with open(file_path, 'w') as file_:
        print " Write mission to file"
        file_.write(output)

    print "print export mission file"
    print_mission_file(file_path)


