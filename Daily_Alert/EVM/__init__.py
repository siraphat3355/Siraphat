import azure.functions as func


from .Do import *


def main(mytimer: func.TimerRequest) -> None:
    utc_timestamp = datetime.datetime.utcnow().replace(
        tzinfo=datetime.timezone.utc).isoformat()

    do()
    
    logging.info('Python timer trigger function ran at %s', utc_timestamp)
