from flask import Flask
from flask import request
from datetime import datetime
from flask import render_template
from mutableint import MutableInt

heartRate = list()
spo2 = list()
readings = list()
stepCount = MutableInt(0)

app = Flask(__name__)
@app.route("/")
def processData():
        heartRate.append(float(request.args.get("hr")))
        spo2.append(float(request.args.get("o2")))
        readings.append(len(heartRate))
        stepCount.set(int(request.args.get("sc")))
        return "Data Transferred."

@app.route("/graph")
def sendGraph():
        return render_template('response.html',
                                times = str(readings),
                                heartRate = str(heartRate),
                                spo2 = str(spo2),
                                steps = str(stepCount))