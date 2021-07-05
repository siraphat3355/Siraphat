import netatmo

# fetch data using ~/.netatmorc credentials
netatmo.fetch()

# credentials as parameters
ws = netatmo.WeatherStation( {
        'client_id': '60db2d1de419af6a20780f80',
        'client_secret': 'MUAr6oYxNtJ8uBFNDlhoTYISKuG9pCkjQG0LU',
        'username': 'siraphat.b@ku.th',
        'password': '@Jj024484055',
        'device': '70:ee:50:19:bf:64' } )