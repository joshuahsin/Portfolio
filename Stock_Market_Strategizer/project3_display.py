#Joshua Hsin
#ID 13651420
#project3_display.py

def display(stockupdate:[{}], indicator_list:[[]]) -> None:
    '''Displays chart with stock info including indicator, buy and sell values'''
    print('Date\tOpen\tHigh\tLow\tClose\tVolume\tIndicator\tBuy?\tSell?')
    for index in range(len(stockupdate)):
        stockupdate[index]['indicator'] = indicator_list[0][index]
        stockupdate[index]['buy'] = indicator_list[1][index]
        stockupdate[index]['sell'] = indicator_list[2][index]
    for day in stockupdate:
        print(day['date'] + '\t' + str(format(day['open'], '.4f')) + '\t' + str(format(day['high'], '.4f')) + '\t' + str(format(day['low'], '.4f')) + '\t' + str(format(day['close'], '.4f'))
        + '\t' + str(day['volume']), end = '')
        if day['indicator'] == '':
            print('\t', end = '')
            if day['buy'] == '':
                print('\t', end = '')
                if day['sell'] == '':
                    print('\t')
                else:
                    print('\t' + day['sell'])
            else:
                print('\t' + day['buy'], end = '')
                if day['sell'] == '':
                    print('\t')
                else:
                    print('\t' + day['sell'])
        elif type(day['indicator']) == float:
            print('\t' + str(format(day['indicator'], '.4f')), end = '')
            if day['buy'] == '':
                print('\t', end = '')
                if day['sell'] == '':
                    print('\t')
                else:
                    print('\t' + day['sell'])
            else:
                print('\t' + day['buy'], end = '')
                if day['sell'] == '':
                    print('\t')
                else:
                    print('\t' + day['sell'])
        elif type(day['indicator']) == int:
            print('\t' + str(day['indicator']), end = '')
            if day['buy'] == '':
                print('\t', end = '')
                if day['sell'] == '':
                    print('\t')
                else:
                    print('\t' + day['sell'])
            else:
                print('\t' + day['buy'], end = '')
                if day['sell'] == '':
                    print('\t')
                else:
                    print('\t' + day['sell'])
    print('Data provided for free by IEX')
    print('View IEX\'s Terms of Use')
    print('https://iextrading.com/api-exhibit-a/')
