#Joshua Hsin ID# 13651420
import time
import json
import ssl
import _pickle as pickle
#https://docs.python.org/3/library/pickle.html
from collections import defaultdict
import os
from nltk.stem import PorterStemmer
#https://pythonprogramming.net/stemming-nltk-tutorial/
import re

doc_num = 1
key = dict()
ps = PorterStemmer()
#https://pythonprogramming.net/stemming-nltk-tutorial/
drop_final_list = [open("a.txt", "rb"), open("b.txt", "rb"), open("c.txt", "rb"), open("d.txt", "rb"), open("e.txt", "rb"), open("f.txt", "rb"),
                   open("g.txt", "rb"), open("h.txt", "rb"), open("i.txt", "rb"), open("j.txt", "rb"), open("k.txt", "rb"), open("l.txt", "rb"),
                   open("m.txt", "rb"), open("n.txt", "rb"), open("o.txt", "rb"), open("p.txt", "rb"), open("q.txt", "rb"), open("r.txt", "rb"),
                   open("s.txt", "rb"), open("t.txt", "rb"), open("u.txt", "rb"), open("v.txt", "rb"), open("w.txt", "rb"), open("x.txt", "rb"),
                   open("y.txt", "rb"), open("z.txt", "rb"), open("extra.txt", "rb")]
stop_words = ['a', 'about', 'above', 'after', 'again', 'against', 'all', 'am', 'an','and', 'any', 'are','aren\'t', 'as'
              , 'at', 'be', 'because', 'been', 'before', 'being', 'below','between', 'both', 'but', 'by', 'can\'t',
              'cannot', 'could', 'couldn\'t', 'did', 'didn\'t', 'do', 'does', 'doesn\'t', 'doing', 'don\'t', 'down', 'during',
              'each', 'few', 'for', 'from', 'further', 'had', 'hadn\'t', 'has', 'hasn\'t', 'have', 'haven\'t', 'having', 'he',
              'he\'d', 'he\'ll', 'he\'s', 'her', 'here', 'here\'s', 'hers', 'herself', 'him', 'himself', 'his', 'how', 'how\'s',
              'i', 'i\'d', 'i\'ll', 'i\'m', 'i\'ve', 'if', 'in', 'into', 'is', 'isn\'t', 'it', 'it\'s', 'its', 'itself', 'let\'s',
              'me', 'more', 'most', 'mustn\'t', 'my', 'myself', 'no', 'nor', 'not', 'of', 'off', 'on', 'once', 'only', 'or',
              'other', 'ought', 'our', 'ours', 'to']

def check(word):
    #check word
    if(re.match(r"([.]{0})([\-]{1,})([.]{0})$", word) or re.match(r"(.)*(https://)(.)*", word) or re.match(r"(.)*(http://)(.)*", word)):
        #get rid of words with multiple dashes and urls
        return ''
    if(re.match(r"([0-9]{1,2})(:{1})([0-5]{1}[0-9]{1})(am|pm)$", word)):
        return ''
    if(re.match(r"([0-9]{1,2})(st|nd|rd|th{1})$", word)):
        return word
    if(re.match(r"(.)*(.edu)$", word) and word != 'uci.edu' and word != 'ics.edu' and word != 'ics.uci.edu'):
        #return emails as is
        return word
    if(re.match(r"(([A-Za-z]{1})(\.{1})){2,}", word)):
        #Get rid of extraneous parts of words if acronym in word, then stem
        word = re.sub(r"[^a-z|^A-Z]", '', word)
        #word = "".join(word.split())
        return word
    word = re.sub(r"[^a-z|^A-Z|^\'|^\-]", ' ', word)
    if(re.match(r"([\']{3})([A-Za-z\-]+)([\']{3})$", word)):
        #if quotations around word, get rid of them
        word = word[3:-3]
        word = ps.stem(word)
        return word
    if(re.match(r"([A-Za-z\-]+)([\']{3})$", word)):
        #if quotations after word, get rid of them
        word = word[:-3]
        word = ps.stem(word)
        return word
    if(re.match(r"([\']{3})([A-Za-z\-]+)$", word)):
        #if quotations before word, get rid of them
        word = word[3:]
        word = ps.stem(word)
        return word
    if(re.match(r"([\']{1})([A-Za-z\-]+)([\']{1})$", word) or re.match(r"([\-]{1})([A-Za-z\']+)([\-]{1})$",word)):
        #if quotations or dashes around word, get rid of them
        word = word[1:-1]
        word = ps.stem(word)
        return word
    if(re.match(r"([A-Za-z\-]+)([\']{1})$", word) or re.match(r"([A-Za-z\']+)([\-]{1})$", word)):
        #if quotations or dashes after word, get rid of them
        word = word[:-1]
        word = ps.stem(word)
        return word
    if(re.match(r"([\']{1})([A-Za-z\-]+)$", word) or re.match(r"([\-]{1})([A-Za-z\']+)$", word)):
        #if quotations or dashes before word, get rid of them
        word = word[1:]
        word = ps.stem(word)
        return word
    if(word[-2] != "\'"):
        word = ps.stem(word)
    return word

def open_files():
    #open files and get key dictionary of document numbers which point to urls
    global doc_num
    ssl._create_default_https_context = ssl._create_unverified_context
    for subdir, dirs, files in os.walk('DEV'):
        #https://stackoverflow.com/questions/19587118/iterating-through-directories-with-python
        print(str(subdir))
        for filename in files:
            if(str(os.path.join(subdir, filename)).endswith('.DS_Store')):
                pass
            else:
                a = open(str(os.path.join(subdir, filename)), 'r')
                
                b = json.load(a)
                key[doc_num] = str(b['url'])
                doc_num += 1

def takequery(query):
    start = time.time()
    #https://stackoverflow.com/questions/7370801/how-to-measure-elapsed-time-in-python
    global key
    global stop_words
    query_list = []
    
    #parse query
    for x in query.split():
        query_list.append(check(x.lower()))
        
    #remove stop words
    remove_list = []
    remove_words = 0
    for x in query_list:
        if(x in stop_words):
            remove_words += 1
            remove_list.append(x)
    if(remove_words < (len(query_list) / 2.0)):
        for x in remove_list:
            query_list.remove(x)
    query_list_len = len(query_list)
    match_list = dict()
    query_match_list = []
    
    #get word dictionaries for all valid query words
    for x in range(query_list_len):
        try:
            first_letter = ord(query_list[x][0]) - 97
            if(0<= first_letter <= 25):
                pass
            else:
                first_letter = -1
            word_list = []
            A = drop_final_list[first_letter]
            b = pickle.load(A)
            while True:
                c = list(b.keys())[0]
                if(type(c) == str):
                    try:
                        if(query_list[x] == c):
                            query_match_list.append(b[c])
                            drop_final_list[first_letter].seek(0)
                            break
                        elif(query_list[x] < c):
                            query_match_list.append(None)
                            drop_final_list[first_letter].seek(0)
                            break
                        b = pickle.load(A)
                    except:
                        query_match_list.append(None)
                        drop_final_list[first_letter].seek(0)
                        break
        except:
            query_match_list.append(None)

    #Get the 20 most relevant urls (if possible)
    query_match_list2 = []
    for x in range(len(query_match_list)):
        if(query_match_list[x] != None):
            query_match_list2.append(query_match_list[x])
    if(len(query_match_list2) == 0):
        print("no results")
        return
    elif(len(query_match_list2) == 1):
        o = query_match_list2[0]
        i = 0
        for x, y in sorted(o.items(), key = lambda item:item[1]):
            match_list[x] = o[x]
            i += 1
            if(i == 20):
                break
    elif(len(query_match_list2) == 2):
        i = 0
        temp_list = dict()
        temp_list2 = dict()
        len_x = 10000000
        check_list = []
        index = -1
        index2 = -1
        if(len(query_match_list2[0]) < len(query_match_list2[1])):
            index = 0
            index2 = 1
        else:
            index = 1
            index2 = 0
        for x in sorted(query_match_list2[index].keys()):
            if(x not in temp_list.keys()):
                if x in query_match_list2[index2].keys():
                    temp_list[x] = query_match_list2[0][x] + query_match_list2[1][x]
                    i += 1
        if(i < 20):
            for x in sorted(query_match_list2[0].keys()):
                if(x not in temp_list.keys()):
                    if x not in query_match_list2[1].keys():
                        temp_list2[x] = query_match_list2[0][x]
            for x in sorted(query_match_list2[1].keys()):
                if(x not in temp_list.keys()):
                    if x not in query_match_list2[0].keys():
                        temp_list2[x] = query_match_list2[1][x]
            o = 0
            for x,y in sorted(temp_list.items(), key = lambda temp_list: temp_list[1]):
                match_list[x] = y
                o += 1
            for x,y in sorted(temp_list2.items(), key = lambda temp_list: temp_list[1]):
                match_list[x] = y
                o += 1
                if(o == 20):
                    break
        else:
            o = 0
            for x,y in sorted(temp_list.items(), key = lambda temp_list: temp_list[1]):
                match_list[x] = y
                o += 1
                if(o == 20):
                    break
    else:
        i = 0
        index = -1
        term_sub = 0
        temp_list = dict()
        len_x = 10000000
        check_list = []
        
        for x in range(len(query_match_list2)):
            if(query_match_list2[x] != None):
                if(len(query_match_list2[x]) < len_x):
                    index = x
                    len_x = len(query_match_list2[x])
        while(i < 20 and term_sub < (len(query_match_list2) - 1)):
            check_list = []
            for x in sorted(query_match_list2[index].keys()):
                if(x not in match_list.keys()):
                    rank = query_match_list2[index][x]
                    num = 1
                    for y in range(len(query_match_list2)):
                        if(y != index):
                            if x in query_match_list2[y].keys():
                                num += 1
                                rank += query_match_list2[y][x]
                                if((num) == len(query_match_list2) - term_sub):
                                    past = False
                                    if(len(check_list) == 20 - i):
                                        a = min(check_list)
                                        if(rank > a):
                                            check_list[check_list.index(a)] = rank
                                            past = True
                                    elif(len(check_list) < 20 - i):
                                        check_list.append(rank)
                                        past = True
                                    if(past == True):
                                        temp_list[x] = rank
            for x,y in sorted(temp_list.items(), key = lambda temp_list: temp_list[1]):
                match_list[x] = y
                i += 1
                if(i == 20):
                    break
            temp_list.clear()
            term_sub += 1
    end = time.time()
    for m, e in sorted(match_list.items(), key = lambda match_list: match_list[1]):
        #https://stackoverflow.com/questions/613183/how-do-i-sort-a-dictionary-by-value
        print(key[m])
    print(end - start, 'seconds')

if __name__ == '__main__':
    open_files()
    print('done')
    a = input('start: ')
    if(a == 'startMyEngine'):
        while True:
            query = input('query: ')
            takequery(query.lower())
