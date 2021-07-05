import datetime
import logging

import azure.functions as func
from main import *


def main(mytimer: func.TimerRequest) -> None:
    utc_timestamp = datetime.datetime.utcnow().replace(
        tzinfo=datetime.timezone.utc).isoformat()


    logging.info('Python timer trigger function ran at %s', utc_timestamp)
