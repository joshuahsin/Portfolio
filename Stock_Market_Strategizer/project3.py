#Joshua Hsin
#ID 13651420
#project3.py
from collections import namedtuple
import urllib.request
import json
import project3_get_info
import project3_display
import project3_strategies
stockinfo = namedtuple('stockinfo', ['Date', 'Open', 'High', 'Low', 'Close', 'Volume', 'Indicator', 'Buy', 'Sell'])

BaseURL = 'https://api.iextrading.com/1.0/stock/'

def stock_input() ->None:
    '''Get's input from user - symbol of stock, trading days, strategy'''
    try:
        symbol = input()
        trading_days = input()
        indicator = input()
        print(symbol)
        compURL = urllib.request.urlopen(BaseURL + symbol + '/company')
        data = compURL.read()
        encode = json.loads(data)
        print(encode['companyName'])
        project3_get_info.get_shares(symbol)
        URL = project3_get_info.build_search_url(symbol, trading_days)
        get_info(URL, indicator)
        
    except urllib.error.HTTPError:
        print('Information not found. Try again')
        print('First input: symbol, second input: trading_days, third input: strategy')
        stock_input()

def calculate_indicator(indicator: str, chart:[{}]) -> [[]]:
    '''Returns list of 3 lists containing indicator, buy and sell values for each trading day'''
    while True:
        indicator1 = indicator.split()
        if indicator1[0] == 'TR':
            try:
                buy_threshold = indicator1[1]
                sell_threshold = indicator1[2]
                if len(indicator1) > 3:
                    print('Too many fields, please give valid input in form TR </># </>#')
                    indicator = input()
                else:
                    if buy_threshold[0] in ['>', '<'] and sell_threshold[0] in ['>','<']:
                        lst = project3_strategies.true_range(buy_threshold, sell_threshold, chart)
                        return lst
                        break
                    else:
                        print('Invalid form, please give valid input in form TR </># </>#')
                        indicator = input()
            except IndexError:
                print('Not enough Information, please give valid input in form TR </># </>#')
                indicator = input()
        elif indicator1[0] == 'MP':
            try:
                if len(indicator1) > 2:
                    print('Too many fields, please give valid input in form TR </># </>#')
                    indicator = input()
                else:
                    lst = project3_strategies.simple_moving_avg(int(indicator1[1]), 'close', chart)
                    return lst
                    break
            except ValueError:
                print('Not an integer, please give valid input in form MP #')
                indicator = input()
            except IndexError:
                print('Second element not given, please give valid input in form MP #')
                indicator = input()
        elif indicator1[0] == 'MV':
            try:
                if len(indicator1) > 2:
                    print('Too many fields, please give valid input in form TR </># </>#')
                    indicator = input()
                else:
                    lst = project3_strategies.simple_moving_avg(int(indicator1[1]),'volume', chart)
                    return lst
                    break
            except ValueError:
                print('Not an integer, please give valid input in form MV #')
                indicator = input()
            except IndexError:
                print('Second element not given, please give valid input in form MV #')
                indicator = input()
        elif indicator1[0] == 'DP':
            try:
                if len(indicator1) > 4:
                    print('Too many fields, please give valid input in form DP # +/-# +/-#')
                    indicator = input()
                else:
                    days = int(indicator1[1])
                    buy_symbol = indicator1[2][0]
                    sell_symbol = indicator1[3][0]
                    if buy_symbol in ['+','-'] and sell_symbol in ['+','-']:
                        buy_indicator_value = int(indicator1[2][1:])
                        sell_indicator_value = int(indicator1[3][1:])
                        lst = project3_strategies.directional_indicator(days, buy_symbol, buy_indicator_value, sell_symbol, sell_indicator_value, chart, 'close')
                        return lst
                        break
                    else:
                        print('Invalid form, please give valid input in form DP # +/-# +/-#')
                        indicator = input()
            except ValueError:
                print('Integer value not given, please give input in form DP # +/-# +/-#')
                indicator = input()
            except IndexError:
                print('Not enough information, please give input in form DP # +/-# +/-#')
                indicator = input()
        elif indicator1[0] == 'DV':
            try:
                if len(indicator1) > 4:
                    print('Too many fields, please give valid input in form DV # +/-# +/-#')
                    indicator = input()
                else:
                    days = int(indicator1[1])
                    buy_symbol = indicator1[2][0]
                    sell_symbol = indicator1[3][0]
                    if buy_symbol in ['+','-'] and sell_symbol in ['+','-']:
                        buy_indicator_value = int(indicator1[2][1:])
                        sell_indicator_value = int(indicator1[3][1:])
                        lst = project3_strategies.directional_indicator(days, buy_symbol, buy_indicator_value, sell_symbol, sell_indicator_value, chart, 'volume')
                        return lst
                        break
                    else:
                        print('Invalid form, please give valid input in form DV # +/-# +/-#')
                        indicator = input()
            except ValueError:
                print('Integer value not given, please give input in form DV # +/-# +/-#')
                indicator = input()
            except IndexError:
                print('Not enough information, please give input in form DV # +/-# +/-#')
                indicator = input()
        else:
            print('Please enter a valid strategy')
            indicator = input()
    
def get_info(URL: str, indicator: list) -> None:
    '''Access's iex data from URL'''
    response = urllib.request.urlopen(URL)
    data = response.read()
    obj = json.loads(data)
    chart1 = obj['chart']
    indicator = calculate_indicator(indicator, chart1)
    project3_display.display(chart1, indicator)

        
if __name__ == '__main__':
    stock_input()


