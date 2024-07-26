#Joshua Hsin ID# 13651420
import json
from json2html import *
from bs4 import BeautifulSoup
from urllib.parse import urlparse
from urllib.request import urlopen
import ssl
import os
import re
import sys
import _pickle as pickle
##https://docs.python.org/3/library/pickle.html
from collections import defaultdict
from nltk.stem import PorterStemmer
#https://pythonprogramming.net/stemming-nltk-tutorial/
import warnings
import math

warnings.filterwarnings("ignore", category=UserWarning, module='bs4')
ps = PorterStemmer()
key = dict() #key dictionary with urls
word_dict = defaultdict(dict) #word_dictionary

drop = open("dropoff.txt", "wb")
drop2 = open("dropoff2.txt", "wb")
drop3 = open("dropoff3.txt", "wb")
drop_final_list = [open("a.txt", "wb"), open("b.txt", "wb"), open("c.txt", "wb"), open("d.txt", "wb"), open("e.txt", "wb"), open("f.txt", "wb"),
                   open("g.txt", "wb"), open("h.txt", "wb"), open("i.txt", "wb"), open("j.txt", "wb"), open("k.txt", "wb"), open("l.txt", "wb"),
                   open("m.txt", "wb"), open("n.txt", "wb"), open("o.txt", "wb"), open("p.txt", "wb"), open("q.txt", "wb"), open("r.txt", "wb"),
                   open("s.txt", "wb"), open("t.txt", "wb"), open("u.txt", "wb"), open("v.txt", "wb"), open("w.txt", "wb"), open("x.txt", "wb"),
                   open("y.txt", "wb"), open("z.txt", "wb"), open("extra.txt", "wb")]
stop_words = ['a', 'about', 'above', 'after', 'again', 'against', 'all', 'am', 'an','and', 'any', 'are','aren\'t', 'as'
              , 'at', 'be', 'because', 'been', 'before', 'being', 'below','between', 'both', 'but', 'by', 'can\'t',
              'cannot', 'could', 'couldn\'t', 'did', 'didn\'t', 'do', 'does', 'doesn\'t', 'doing', 'don\'t', 'down', 'during',
              'each', 'few', 'for', 'from', 'further', 'had', 'hadn\'t', 'has', 'hasn\'t', 'have', 'haven\'t', 'having', 'he',
              'he\'d', 'he\'ll', 'he\'s', 'her', 'here', 'here\'s', 'hers', 'herself', 'him', 'himself', 'his', 'how', 'how\'s',
              'i', 'i\'d', 'i\'ll', 'i\'m', 'i\'ve', 'if', 'in', 'into', 'is', 'isn\'t', 'it', 'it\'s', 'its', 'itself', 'let\'s',
              'me', 'more', 'most', 'mustn\'t', 'my', 'myself', 'no', 'nor', 'not', 'of', 'off', 'on', 'once', 'only', 'or',
              'other', 'ought', 'our', 'ours']
doc_num = 1

def drop_word(d, word, value):
    #dump a word dictionary into appropriate file
    global doc_num
    for x in d[word].keys():
        if(d[word][x] > 0):
            d[word][x] *= math.log(doc_num/len(d[word].keys()))
    pickle.dump(d, drop_final_list[value])

def merge1(drop, a):
    #merge the rest of one file
    global doc_num
    finish = False
    while not finish:
        try:
            a_word = list(a.keys())[0]
            for x in a[a_word].keys():
                a[a_word][x] *= math.log(doc_num/len(a[a_word].keys()))
            if(0 <= (ord(a_word[0]) - 97) <= 25):
                pickle.dump(a,drop_final_list[ord(a_word[0]) - 97])
            else:
                pickle.dump(a,drop_final_list[-1])
            a = pickle.load(drop)
        except EOFError:
            finish = True
    return

def merge2(drop, drop2, a, b):
    #merge two files together
    a_word = list(a.keys())[0]
    b_word = list(b.keys())[0]
    finish = False
    while not finish:
        if(a_word < b_word):
            try:
                value = ord(a_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(a,a_word, value)
                else:
                    drop_word(a,a_word, -1)
                a = pickle.load(drop)
                a_word = list(a.keys())[0]
            except EOFError:
                merge1(drop2, b)
                finish = True
        elif(a_word == b_word):
            d = defaultdict(dict)
            d[a_word] = {**a[a_word], **b[b_word]}
            value = ord(a_word[0]) - 97
            if(0 <= value <= 25):
                drop_word(d,a_word, value)
            else:
                drop_word(d,a_word, -1)
            try:
                a = pickle.load(drop)
                a_word = list(a.keys())[0]
                try:
                    b = pickle.load(drop)
                    b_word = list(b.keys())[0]
                except EOFError:
                    merge1(drop, a)
                    finish = True
            except EOFError:
                try:
                    b = pickle.load(drop)
                    merge1(drop2, b)
                    finish = True
                except EOFError:
                    finish = True
        elif(a_word > b_word):
            #print(a,b)
            try:
                value = ord(b_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(b,b_word, value)
                else:
                    drop_word(b,b_word, -1)
                b = pickle.load(drop2)
                b_word = list(b.keys())[0]
            except EOFError:
                merge1(drop,a)
                finish = True
    return

def merge():
    #merge all 3 files together
    drop = open('dropoff.txt', 'rb')
    drop2 = open('dropoff2.txt', 'rb')
    drop3 = open('dropoff3.txt', 'rb')
    a = pickle.load(drop)
    b = pickle.load(drop2)
    c = pickle.load(drop3)
    a_word = list(a.keys())[0]
    b_word = list(b.keys())[0]
    c_word = list(c.keys())[0]
    finish = False
    while not finish:
        #compare all three files to determine which has the lowest value word
        #dump the lowest value word into the appropriate file (a - z and extra)
        #if files have the same word, merge word dicts, then dump word_dict
        if(a_word < b_word):
            if(a_word < c_word):
                value = ord(a_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(a,a_word, value)
                else:
                    drop_word(a,a_word, -1)
                try:
                    a = pickle.load(drop)
                    a_word = list(a.keys())[0]
                except EOFError:
                    merge2(drop2, drop3, b, c)
                    finish = True
            elif(a_word == c_word):
                d = defaultdict(dict)
                d[a_word] = {**a[a_word], **c[c_word]}
                value = ord(a_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(d,a_word, value)
                else:
                    drop_word(d,a_word, -1)
                try:
                    a = pickle.load(drop)
                    a_word = list(a.keys())[0]
                    try:
                        c = pickle.load(drop3)
                        c_word = list(c.keys())[0]
                    except EOFError:
                        merge2(drop, drop2, a, b)
                        finish = True
                except EOFError:
                    try:
                        c = pickle.load(drop3)
                        merge2(drop2, drop3, b, c)
                        finish = True
                    except EOFError:
                        merge1(drop2, b)
                        finish = True
            elif(c_word < a_word):
                #print('3',a_word,b_word,c_word)
                value = ord(c_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(c,c_word, value)
                else:
                    drop_word(c,c_word, -1)
                try:
                    c = pickle.load(drop3)
                    c_word = list(c.keys())[0]
                except EOFError:
                    merge2(drop, drop2, a, b)
                    finish = True
        elif(a_word == b_word):
            if(b_word < c_word):
                d = defaultdict(dict)
                d[a_word] = {**a[a_word], **b[b_word]}
                value = ord(a_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(d,a_word, value)
                else:
                    drop_word(d,a_word, -1)
                try:
                    a = pickle.load(drop)
                    a_word = list(a.keys())[0]
                    try:
                        b = pickle.load(drop2)
                        b_word = list(b.keys())[0]
                    except EOFError:
                        merge2(drop, drop3, a, c)
                        finish = True
                except EOFError:
                    try:
                        b = pickle.load(drop2)
                        merge2(drop2, drop3, b, c)
                        finish = True
                    except EOFError:
                        merge1(drop3, c)
                        finish = True
            elif(b_word == c_word):
                d = defaultdict(dict)
                d[a_word] = {**a[a_word], **b[b_word], **c[c_word]}
                value = ord(a_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(d,a_word, value)
                else:
                    drop_word(d,a_word, -1)
                try:
                    a = pickle.load(drop)
                    try:
                        b = pickle.load(drop2)
                        try:
                            c = pickle.load(drop3)
                        except EOFError:
                            merge2(drop, drop2, a, b)
                            finish = True
                    except EOFError:
                        try:
                            c = pickle.load(drop3)
                            merge2(drop, drop2, a, c)
                            finish = True
                        except EOFError:
                            merge1(drop,a)
                            finish = True
                except EOFError:
                    try:
                        b = pickle.load(drop2)
                        try:
                            c = pickle.load(drop3)
                            merge2(drop2, drop3, b, c)
                            finish = True
                        except EOFError:
                            merge1(drop3, b)
                            finish = True
                    except EOFError:
                        try:
                            c = pickle.load(drop3)
                            merge1(drop3, c)
                            finish = True
                        except EOFError:
                            finish = True
                a_word = list(a.keys())[0]
                b_word = list(b.keys())[0]
                c_word = list(c.keys())[0]
            elif(b_word > c_word):
                value = ord(c_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(c,c_word, value)
                else:
                    drop_word(c,c_word, -1)
                try:
                    c = pickle.load(drop3)
                    c_word = list(c.keys())[0]
                except EOFError:
                    merge2(drop, drop2, a, b)
                    finish = True
        elif(b_word < a_word):
            if(b_word < c_word):
                value = ord(b_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(b,b_word, value)
                else:
                    drop_word(b,b_word, -1)
                try:
                    b = pickle.load(drop2)
                    b_word = list(b.keys())[0]
                except EOFError:
                    try:
                        c = pickle.load(drop3)
                        merge2(drop, drop3, a, c)
                        finish = True
                    except EOFError:
                        merge1(drop, a)
                        finish = True
            elif(b_word == c_word):
                d = defaultdict(dict)
                d[b_word] = {**b[b_word], **c[c_word]}
                value = ord(b_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(d,b_word, value)
                else:
                    drop_word(d,b_word, -1)
                try:
                    b = pickle.load(drop2)
                    b_word = list(b.keys())[0]
                    try:
                        c = pickle.load(drop3)
                        c_word = list(c.keys())[0]
                    except:
                        merge2(drop, drop2, a, b)
                        finish = True
                except EOFError:
                    try:
                        c = pickle.load(drop3)
                        merge2(drop, drop3, a, c)
                        finish = True
                    except EOFError:
                        merge1(drop, a)
                        finish = True
            elif(b_word > c_word):
                value = ord(c_word[0]) - 97
                if(0 <= value <= 25):
                    drop_word(c,c_word, value)
                else:
                    drop_word(c,c_word, -1)
                try:
                    c = pickle.load(drop3)
                    c_word = list(c.keys())[0]
                except EOFError:
                    merge2(drop, drop2, a, b)
                    finish = True
    return

def drop_off3(word_dict):
    #drop off current word_dict into drop3 file
    global drop3
    for word in sorted(word_dict.keys()):
        a = defaultdict(dict)
        a[word] = word_dict[word]
        for x,y in a[word].items():
            if(a[word][x] > 0):
                a[word][x] = (1 + math.log(a[word][x], 10))
        pickle.dump(a, drop3)
    return

def drop_off2(word_dict):
    #drop off current word_dict into drop2 file
    global drop2
    for word in sorted(word_dict.keys()):
        a = defaultdict(dict)
        a[word] = word_dict[word]
        for x,y in a[word].items():
            if(a[word][x] > 0):
                a[word][x] = (1 + math.log(a[word][x], 10))
        pickle.dump(a, drop2)
    return

def drop_off(word_dict):
    #drop off current word_dict into drop file
    global drop
    for word in sorted(word_dict.keys()):
        a = defaultdict(dict)
        a[word] = word_dict[word]
        for x,y in a[word].items():
            if(a[word][x] > 0):
                a[word][x] = (1 + math.log(a[word][x], 10))
        pickle.dump(a, drop)
    return

def short_word(word):
    #if word is <= 1 character, not a or i, and not a number return empty word
    if(len(word) <= 1 and (word != 'a' and word != 'i' and word != 'A' and word != 'I' and word not in ['0','1','2','3','4','5','6','7','8','9'])):
        word = ''
    elif(len(word) > 20):
        word = ''
    return word

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
        #word = "".join(word.split())
        return word
    if(re.match(r"([\']{1})([A-Za-z\-]+)$", word) or re.match(r"([\-]{1})([A-Za-z\']+)$", word)):
        #if quotations or dashes before word, get rid of them
        word = word[1:]
        word = ps.stem(word)
        return word
    if(word[-2] != "\'"):
        word = ps.stem(word)
    return word

def insert(word, value):
    #insert words and values into word_dict
    #if word already exists, increment value of appropriate document for word
    global word_dict
    global doc_num
    if word != '':
        if ' ' in word:
            words = word.split()
            for i in words:
                i = check(i)
                if i != '':
                    if(len(i) != 1 or i in ['0','1','2','3','4','5','6','7','8','9']):
                        if i not in word_dict.keys():
                            word_dict[i][doc_num] = value
                        else:
                            if(doc_num not in word_dict[i].keys()):
                                word_dict[i][doc_num] = value
                            else:
                                word_dict[i][doc_num] += value
        else:
            if(word not in word_dict.keys()):
                word_dict[word][doc_num] = value
            else:
                if(doc_num not in word_dict[word].keys()):
                    word_dict[word][doc_num] = value
                else:
                    word_dict[word][doc_num] += value
    return word_dict

def parse(soup, tag, value):
    #parse through the json file words
    global word_dict
    a = soup.find_all(tag)
    for x in range(len(a)):
        i = a[x].text
        for word2 in i.split():
            word = check(word2.lower())
            word_dict = insert(word, value)
    return word_dict

def index():
    #index words into inverted index
    global key
    global doc_num
    global word_dict
    directory_num = 1
    
    ssl._create_default_https_context = ssl._create_unverified_context
    #https://stackoverflow.com/questions/27835619/urllib-and-ssl-certificate-verify-failed-error
    for subdir, dirs, files in os.walk('DEV'):
        #https://stackoverflow.com/questions/19587118/iterating-through-directories-with-python
        print(str(subdir))
        for filename in files:
            if(str(os.path.join(subdir, filename)).endswith('.DS_Store')):
                #ignore extraneous files (for Mac)
                pass
            else:
                #load files, put url into key dictionary
                a = open(str(os.path.join(subdir, filename)), 'r')
                b = json.load(a)
                key[doc_num] = str(b['url'])
                #https://pypi.org/project/json2html/
                try:
                    #find tags with words and index
                    soup = BeautifulSoup(b['content'], 'lxml')
                    #https://www.crummy.com/software/BeautifulSoup/bs4/doc/
                    if(soup.find('title') != None):
                        word_dict = parse(soup, 'title', 1.5)
                    if(soup.find('p') != None):
                        word_dict = parse(soup, 'p', 1)
                    if(soup.find('h1') != None):
                        word_dict = parse(soup, 'h1', 1.5)
                    if(soup.find('h2') != None):
                        word_dict = parse(soup, 'h2', 1.5)
                    if(soup.find('h3') != None):
                        word_dict = parse(soup, 'h3', 1.5)
                    if(soup.find('h4') != None):
                        word_dict = parse(soup, 'h4', 1)
                    if(soup.find('h5') != None):
                        word_dict = parse(soup, 'h5', 1)
                    if(soup.find('h6') != None):
                        word_dict = parse(soup, 'h6', 1)
                    if(soup.find('b') != None):
                        word_dict = parse(soup, 'b', 1.5)
                    if(soup.find('strong') != None):
                        word_dict = parse(soup, 'strong', 1.5)
                    if(soup.find('i') != None):
                        word_dict = parse(soup, 'i', 1.5)
                    if(soup.find('u') != None):
                        word_dict = parse(soup, 'u', 1.5)
                    doc_num += 1
                except:
                    pass
        if(directory_num == 29):
            drop_off(word_dict)
            global drop
            drop.close()
            word_dict.clear()
        elif(directory_num == 59):
            drop_off2(word_dict)
            global drop2
            drop2.close()
            word_dict.clear()
        elif(directory_num == 89):
            drop_off3(word_dict)
            global drop3
            drop3.close()
            merge()
            break
        directory_num += 1
    #close files
    drop.close()
    drop2.close()
    drop3.close()
    for x in range(len(drop_final_list)):
        drop_final_list[x].close()
    
    for x in range(0, 26):
        drop_final_list[x] = open(chr(x + 97) + '.txt', 'rb')
    drop_final_list[-1] = open('extra.txt', 'rb')

if __name__ == '__main__':
    index()
    print('done')
