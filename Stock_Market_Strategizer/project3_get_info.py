#Joshua Hsin
#ID 13651420
#project3_get_info.py
import urllib.request
import json
BaseURL = 'https://cloud.iexapis.com/stable/stock/'
def build_search_url(symbol:str, days:str) -> str:
    '''Constructs iex URL from user input'''
    query_parameters = [
        ('types', 'quote' + ',' + 'chart'),('range', '5y'),('chartLast', days)
    ]
    return BaseURL + symbol.lower() + '/batch?' + urllib.parse.urlencode(query_parameters)

def get_shares(symbol: str) -> None:
    '''Get's number of shares oustanding for a stock'''
    sharesURL = urllib.request.urlopen(BaseURL + symbol.lower() + '/stats')
    data = sharesURL.read()
    encode = json.loads(data)
    print(encode['sharesOutstanding'])
