import gps
session = gps() # assuming gpsd running with default options on port 2947
session.stream(WATCH_ENABLE|WATCH_NEWSTYLE)
report = session.next()
print(report)