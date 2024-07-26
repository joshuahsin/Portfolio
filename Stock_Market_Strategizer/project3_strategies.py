#Joshua Hsin
#ID 13651420
#project3_strategies.py

def simple_moving_avg(indicator: int, volume_or_closing: str, chart: [{}]) -> [[]]:
    '''Returns list of indicator, buy, and sell values based on simple moving
    average stategy using closing price or volume'''
    list2 = []
    buy = []
    sell = []
    N = indicator
    for i in range(N - 1):
        buy.append('')
        sell.append('')
        list2.append('')
    for i in range((N-1), len(chart)):
        sum1 = 0
        for b in range(i, (i-N), -1):
            sum1 += chart[(b)][volume_or_closing]
        list2.append(round((sum1 / N), 4))
    newlist2 = list2[(N-1):]
    new_chart = chart[(N-1):]
    for element in range(len(newlist2)):
        if element == 0:
            if newlist2[element] > new_chart[element][volume_or_closing]:
                over = True
                buy.append('')
                sell.append('')
            else:
                over = False
                buy.append('')
                sell.append('')
        if over == True:
            if newlist2[element] < new_chart[element][volume_or_closing]:
                over = False
                buy.append('BUY')
                sell.append('')
            else:
                buy.append('')
                sell.append('')
        elif over == False:
            if newlist2[element] > new_chart[element][volume_or_closing]:
                over = True
                buy.append('')
                sell.append('SELL')
            else:
                buy.append('')
                sell.append('')
    del buy[0]
    del sell[0]
    return [list2, buy, sell]


def true_range(buy_threshold: str, sell_threshold: str, chart: [{}]) -> [[]]:
    '''Returns list of indicator, buy, and sell values based on true range stategy'''
    list2 = []
    for element in range(len(chart)):
        list1 = []
        if chart[element] == chart[0]:
            pass
        else:
            previous_close = chart[element - 1]['close']
            list1.append(chart[element]['high'])
            list1.append(chart[element]['low'])
            list1.append(previous_close)
            true_range = max(list1) - min(list1)
            true_range_percent = round((true_range / list1[2]) * 100, 4)
            list2.append(true_range_percent)
    buy = ['']
    sell = ['']
    for element in list2:
        if buy_threshold[0] == '<':
            if float(buy_threshold[1:]) > float(element):
                buy.append('BUY')
            else:
                buy.append('')
        elif buy_threshold[0] == '>':
            if float(buy_threshold[1:]) < float(element):
                buy.append('BUY')
            else:
                buy.append('')
        
    for element in list2:
        if sell_threshold[0] == '<':
            if float(sell_threshold[1:]) > float(element):
                sell.append('SELL')
            else:
                sell.append('')
        elif sell_threshold[0] == '>':
            if float(sell_threshold[1:]) < float(element):
                sell.append('SELL')
            else:
                sell.append('')
    list2.insert(0, '')
    print(list2)
    return [list2, buy, sell]


def directional_indicator(days: int, buy_symbol: str, buy_indicator_value: int, sell_symbol: str,
                          sell_indicator_value: int, chart: [{}], close_or_volume:str) -> [[]]:
    '''Returns list of indicator, buy, and sell values based on directional indicator
    stategy using closing price or volume'''
    list2 = []
    buy = []
    sell = []
    for i in range(len(chart)):
        stock_increase_decrease_count = 0
        if (i - days) < 0:
            for iterable in range(i, -1, -1):
                if chart[iterable][close_or_volume] == chart[0][close_or_volume]:
                    break
                elif chart[iterable][close_or_volume] < chart[iterable - 1][close_or_volume]:
                    stock_increase_decrease_count -= 1
                elif chart[iterable][close_or_volume] > chart[iterable - 1][close_or_volume]:
                    stock_increase_decrease_count += 1
        else:
            for iterable in range(i, i - days, -1):
                if chart[iterable][close_or_volume] < chart[iterable - 1][close_or_volume]:
                    stock_increase_decrease_count -= 1
                elif chart[iterable][close_or_volume] > chart[iterable - 1][close_or_volume]:
                    stock_increase_decrease_count += 1
        list2.append(stock_increase_decrease_count)
        
    for element in list2:
        element = int(element)
        if buy_symbol == '+':
            if element <= buy_indicator_value:
                buy.append('')
            elif element > buy_indicator_value:
                buy.append('BUY')
        elif buy_symbol == '-':
            if element <= (buy_indicator_value * -1):
                buy.append('')
            elif element > (buy_indicator_value * -1):
                buy.append('BUY')
        if sell_symbol == '+':
            if element >= sell_indicator_value:
                sell.append('')
            elif element < sell_indicator_value:
                sell.append('SELL')
        elif sell_symbol == '-':
            if element < (sell_indicator_value * -1):
                sell.append('SELL')
            elif element >= (sell_indicator_value * -1):
                sell.append('')
    return [list2, buy, sell]
