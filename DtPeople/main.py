import datetime
from datetime import timedelta
from io import BytesIO

import numpy as np
from azure.storage.blob import BlobClient, BlobServiceClient, ContainerClient
from line_notify import LineNotify
from sklearn.mixture import GaussianMixture

from config_netatmo import *

ACCESS_TOKEN = "ruqitvgHdqSdlxregyLX6aT5956wrmexnyX5jv9PA0w"

notify = LineNotify(ACCESS_TOKEN)


class azure_manage():  
    def __init__(self, connection_str, container_name):
        self.connection_str = connection_str
        self.container_name = container_name
        self.container_client = ContainerClient.from_connection_string(conn_str=self.connection_str, container_name=self.container_name)
        print("'azure_manage' object is creted")

    def get_all_blobs(self):
        """
        return list of blob names
        """
        blob_list = self.container_client.list_blobs()
        name_list = [blob['name'] for blob in blob_list]
        return name_list
 
    def push_data(self, azure_save_path, local_file_path):
        """
        push a local data to azure blob storage
 
        parameters
        ----------
        azure_save_path : the storage save path that want to store the file
        local_file_path : the local file path that want to upload to storage
 
        return
        ----------
        True : if the action is complete
        False : if the action is incomplete
 
        """
        try:
            # create BlobClient
            blob_client = BlobClient.from_connection_string(conn_str=self.connection_str, container_name=self.container_name, blob_name=azure_save_path)
 
            # upload data to storage
            with open(local_file_path, "rb") as data:
                blob_client.upload_blob(data)
 
            print('[blob] push "{}" to "{}" completed'.format(local_file_path, azure_save_path))
            return True
        except:
            print("[blob] can't push '{}' to '{}'".format(local_file_path, azure_save_path))
            blob_list = self.get_all_blobs()
            if (azure_save_path in blob_list):
                print("error: blob name already exist")
            else:
                print("error: invalid input")
            return False

    def push_encoded_image(self, path, img): #Numpy array.
        """
        upload numpy-array image to blob
        """
        encoded_img = cv2.imencode(".jpg", img)[1].tobytes()
        self.container_client.upload_blob(name=path, data=encoded_img)
 
    def pull_data(self, azure_target_path, local_file_path):
        """
        pull data from azure blob storage to local
 
        parameters
        ----------
        azure_target_path : the storage target path that want to download 
        local_file_path : the local file path that want to store the downloaded file
 
        return
        ----------
        True : if the action is complete
        False : if the action is incomplete
 
        """
        try:
            # create BlobClient
            blob_client = BlobClient.from_connection_string(conn_str=self.connection_str, container_name=self.container_name, blob_name=azure_target_path)
 
            # download data to local
            with open(local_file_path, "wb") as data:
                blob_data = blob_client.download_blob().readall()
                data.write(blob_data)
 
            print('[blob] pull "{}" to "{}" completed'.format(azure_target_path, local_file_path))
            return True
        except:
            print("[blob] can't pull '{}' to '{}'".format(azure_target_path, local_file_path))
            blob_list = self.get_all_blobs()
            if (azure_target_path not in blob_list):
                print("error: blob name doesn't exist")
            else:
                print("error: invalid input")
            return False


connection_str = 'DefaultEndpointsProtocol=https;AccountName=joblove;AccountKey=6vRDCWJb8CfB1FAPvnUlubd4Kcxa75Mtut38aja0R2u293V/JucMzw72lR1aslcJwr2EYQWtlO4ybSAzd4K8fA==;EndpointSuffix=core.windows.net'
container_name = 'joblove'
azureMan = azure_manage(connection_str, container_name)

azure_target_path = "new_gmm_means.npy"
blob_client = BlobClient.from_connection_string(connection_str, container_name,azure_target_path)
blob_data = blob_client.download_blob().readall()
load_bytes = BytesIO(blob_data)
means = np.load(load_bytes, allow_pickle=True)

azure_target_path = "new_gmm_covariances.npy"
blob_client = BlobClient.from_connection_string(connection_str, container_name,azure_target_path)
blob_data = blob_client.download_blob().readall()
load_bytes = BytesIO(blob_data)
covar = np.load(load_bytes, allow_pickle=True)

azure_target_path = "new_gmm_weights.npy"
blob_client = BlobClient.from_connection_string(connection_str, container_name,azure_target_path)
blob_data = blob_client.download_blob().readall()
load_bytes = BytesIO(blob_data)
weights_ = np.load(load_bytes, allow_pickle=True)


gmm = GaussianMixture(n_components = len(means), covariance_type='full')
gmm.precisions_cholesky_ = np.linalg.cholesky(np.linalg.inv(covar))
gmm.weights_ = weights_
gmm.means_ = means
gmm.covariances_ = covar


ws.get_data()
data =ws.devices[0]['dashboard_data']


timestamp = datetime.datetime.fromtimestamp(data['time_utc']) + timedelta(hours=7)

msg = f"\nDatetime    : {timestamp.strftime('%Y-%m-%d %H:%M:%S')} \
        \nTemperature : {data['Temperature']} \
        \nCO2         : {data['CO2']} \
        \nHumidity    : {data['Humidity']} \
        \nNoise       : {data['Noise']} \
        "

x = np.array([data['Temperature'], data['Humidity'], data['CO2'], data['Noise']]).reshape(1,-1)
if gmm.predict(x)[0] == 0:
    msg += '\nNo people in the room'
else:
    msg += '\nPeople in the room'

notify.send(msg)
