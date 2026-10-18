import json
import random
import sys

stations_count = int(sys.argv[1]) if len(sys.argv) > 1 else 10
pipes_count = int(sys.argv[2]) if len(sys.argv) > 2 else 20
path = sys.argv[3] if len(sys.argv) > 3 else "data/mock.json"

names = ["Moscow", "Tver", "Kazan", "Samara", "Ufa", "Perm", "Omsk", "Tomsk", "Orel", "Kursk", "Penza", "Kirov"]
stations = []
for id in range(1, stations_count + 1):
    workshops = random.randint(1, 10)
    stations.append([id, {
        "id": id,
        "name": f"{random.choice(names)} KS-{id}",
        "workshops": workshops,
        "working": random.randint(0, workshops),
        "stationClass": random.randint(1, 5),
    }])

pipes = []
for id in range(1, pipes_count + 1):
    start, end = 0, 0
    if stations_count > 1 and random.random() < 0.7:
        start, end = sorted(random.sample(range(1, stations_count + 1), 2))
    pipes.append([id, {
        "id": id,
        "name": f"km {random.randint(1, 3000)}",
        "length": round(random.uniform(5, 300), 1),
        "diameter": random.choice([500, 700, 1000, 1400]),
        "inRepair": random.random() < 0.2,
        "from": start,
        "to": end,
    }])

with open(path, "w") as file:
    json.dump({"pipes": pipes, "stations": stations}, file, indent=2)
print(f"{path}: {stations_count} stations, {pipes_count} pipes")
