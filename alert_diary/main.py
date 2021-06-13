import numpy as np
import pandas as pd
import plotly.graph_objects as go
from azure.cosmos import CosmosClient
from line_notify import LineNotify
import pendulum

# CosmosDB
cosmos_url = "https://altocosmos.documents.azure.com:443/"
cosmos_key = "W6BRohNWpaaOFAyFMxW5NPGuHCDUHTNwQwXDVOyvMeDxLEtKBE8oF62GoWH2j4xUa3td5jIh6ljt8EVf5lmNdw=="
client = CosmosClient(cosmos_url, credential=cosmos_key)
database_name = 'altonucminteldb'
database = client.get_database_client(database_name)
container_name = 'iotcontainer'
container = database.get_container_client(container_name)


# Get Sensor Data
def get_dataset_from_cosmosdb_sensor():
    # Get dataset from cosmosdb
    try:

        query_string = f"""SELECT *
                               FROM c
                               WHERE c.gatewayid='altonucmintelmonitor' AND c.type='environment'  """ + \
                       """ AND c._ts> """ + str(ts) + """ ORDER BY c._ts DESC """
        results = []
        try:
            for item in container.query_items(
                    query=query_string,
                    enable_cross_partition_query=True):
                results.append(item)
        except Exception as e:
            print("no data")
            print(e)

        return results
    except Exception as e:
        print(f"[Error] get dataset from cosmosdb function: {e}")
def prepare_data_1(data):
    df = pd.DataFrame(data)

    # rename column
    df = df.rename(columns={'timestamp': 'datetime'})
    df.drop(['device_id', 'subdevice_idx', 'subdevice_name', 'luminosity_sub',
             'air_quality_sub', 'noise_sub', 'type', 'device_name', 'gatewayid',
             'id', '_rid', '_self', '_etag', '_attachments', '_ts'], axis=1, inplace=True)

    df['datetime'] = df['datetime'].map(lambda x: pd.to_datetime(x, yearfirst=True))
    df = df.set_index('datetime')
    df.index = df.index.tz_convert('Asia/Bangkok')
    df['date'] = df.index.date
    df['time'] = df.index.time
    df.index = pd.to_datetime(df.date.astype(str) + ' ' + df.time.astype(str))
    del df['date']
    del df['time']
    df['location'] = df['location'].str.replace('room_', '')
    df['location'] = df['location'].str.replace('/iot_devices', '')
    df['temperature'] = pd.to_numeric(df['temperature'])
    df['location'] = pd.to_numeric(df['location'])

    return df.sort_index()

# Get AC Data
def get_dataset_from_cosmosdb_ac():
    # Get dataset from cosmosdb
    try:
        query_string = f"""SELECT c.timestamp, c.device_id, c.subdevice_idx, c.location, c.mode, c.temperature, c.fan, c.room_temperature,
                               c.subdevice_name, c.device_name
                               FROM c
                               WHERE c.gatewayid='altonucmintelmonitor' AND c.type='ac'  """ + \
                       """ AND c._ts> """ + str(ts) + """ ORDER BY c._ts DESC """
        results = []

        try:
            for item in container.query_items(
                    query=query_string,
                    enable_cross_partition_query=True):
                results.append(item)
        except Exception as e:
            print("no data")
            print(e)

        return results
    except Exception as e:
        print(f"[Error] get dataset from cosmosdb function: {e}")
def prepare_data_2(data):
    df = pd.DataFrame(data)

    # rename column
    df = df.rename(columns={'timestamp': 'datetime'})
    df.drop(['device_id', 'subdevice_idx', 'subdevice_name', 'subdevice_name',
             'device_name'], axis=1, inplace=True)

    df['datetime'] = df['datetime'].map(lambda x: pd.to_datetime(x, yearfirst=True))
    df = df.set_index('datetime')
    df.index = df.index.tz_convert('Asia/Bangkok')
    df['date'] = df.index.date
    df['time'] = df.index.time
    df.index = pd.to_datetime(df.date.astype(str) + ' ' + df.time.astype(str))
    del df['date']
    del df['time']
    df['location'] = df['location'].str.replace('room_', '')
    df['location'] = df['location'].str.replace('/iot_devices', '')
    df[df['temperature'] == 'active'] = np.nan
    df[df['temperature'] == 'inactive'] = np.nan
    df = df.dropna()
    df['temperature'] = pd.to_numeric(df['temperature'])
    df['location'] = pd.to_numeric(df['location'])
    df = df[df['mode'] == 'cool']

    return df.sort_index()

# Main
ttoday  = pendulum.today()
sttoday = str(ttoday)[:10]
ts = np.round(ttoday.timestamp())

results = get_dataset_from_cosmosdb_sensor()
results_2 = get_dataset_from_cosmosdb_ac()

df1 = prepare_data_1(results)
df2 = prepare_data_2(results_2)

df1 = df1.loc[sttoday]
df2 = df2.loc[sttoday]

#Join both of them
rr  = pd.merge_asof(df1, df2, left_index = True, right_index=True, by="location", tolerance=pd.Timedelta("10m"))
Ans = rr.dropna()
Ans.drop(['temperature_y', 'room_temperature'], axis = 1, inplace = True)
Ans = Ans.rename(columns={'temperature_x': 'temperature'})
Ans = Ans.groupby('location').resample('1D').mean()

#Show Plotly
# Psychrometric Chart

left_temp  = [21.8, 22.2, 22.5, 22.8, 23.1, 23.6, 24, 24.4, 24.9, 25.5, 26.0]
left_humid = np.arange(100,-10,-10)

right_temp  = [26.5, 27, 27.4, 27.9, 28.4, 29.0, 29.5, 30.2, 30.8, 31.5, 32.3]
right_humid = left_humid

XxX = np.round(Ans.temperature.values,1)
YyY = np.round(Ans.humidity.values,1)

i = 0
tmp = 0
status = []
for num in YyY:
    
  tmpd = (left_temp[(num < left_humid).sum()] + left_temp[(num < left_humid).sum()+1]) / 2.0
  tmpu = right_temp[(num <= right_humid).sum()]
  if XxX[i] < tmpd:
    tmp = 0
  elif XxX[i] > tmpu:
    tmp = 2
  else:
    tmp = 1
  i += 1
  status.append(tmp)

fig2 = go.Figure()

fig2.add_trace(
    go.Scatter(
        x=left_temp,
        y=left_humid,
        fill=None,
        mode='lines',
        line_color='rgb(0,185,255)',
        showlegend=False,
    )
)
fig2.add_trace(
    go.Scatter(
        x=right_temp,
        y=right_humid,
        fill='tonexty',
        mode='lines',
        line_color='rgb(0,185,255)',
        showlegend=False,
    )
)

for _ in range(len(XxX)):
    if status[_] == 0:
        cc = 'rgb(0,0,255)'
    elif status[_] == 1:
        cc = 'rgb(0,255,0)'
    else:
        cc = 'rgb(255,0,0)'

    fig2.add_trace(
        go.Scatter(
            x=[XxX[_]],
            y=[YyY[_]],
            line_color=cc,
            name=str(Ans.location.unique()[_]),
            showlegend=True,
            marker=dict(size=10),
        )
    )

stitle = '<b> Mintel</b> : Average Human Comfort [ ' + str(sttoday) + ' ]'
fig2.update_layout(
    title_text=stitle,
    title={'x': 0.5},
    title_font_size=15,
    xaxis_title="<b>Dry-bulb Temperature [°C]<b>",
    yaxis_title="<b>Relative Humidity [%]<b>",
)

fig2.add_trace(go.Scatter(
    x=[15, 25, 32],
    y=[90, 90, 90],
    mode="text",
    text=["Cold", "Comfort", "Hot"],
    textposition="bottom center",
    textfont=dict(
        family="sans serif",
        size=25,
        color=['rgb(0,0,255)', 'rgb(0,255,0)', 'rgb(255,0,0)']
    ),
    showlegend=False,
))

fig2.update_xaxes(range=[10, 36], nticks=20)
fig2.update_yaxes(range=[0, 100], nticks=20)
fig2.write_image("fig1.jpeg")
fig2.show()


ACCESS_TOKEN = "ruqitvgHdqSdlxregyLX6aT5956wrmexnyX5jv9PA0w"

notify = LineNotify(ACCESS_TOKEN)
notify.send("Image test", image_path='fig1.jpeg')