#! /usr/bin/env python
# -*- coding:gb2312 -*-

import re
import urllib2, urllib
import cookielib
import sys

class NewSmthSession(object):
  def __init__(self, user, password, proxy="",is2site=False):
    self.user=user
    self.password=password
    self.proxy=proxy
    self.is2site=is2site
    self.cookie=cookielib.CookieJar()
    if self.proxy:
      self.opener=urllib2.build_opener(urllib2.ProxyHandler({'http':proxy}),
          urllib2.HTTPCookieProcessor(self.cookie))
    else:
      self.opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(self.cookie))
      
    self.opener.addheaders = [('User-agent', "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1;"
       " .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022;"
       " .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.1;"
       " .NET CLR 1.1.4322; SE 2.X MetaSr 1.0)")]

    if is2site:
      self.login_url='http://www.2.newsmth.net/bbslogin.php'
      self.logout_url='http://www.2.newsmth.net/bbslogout.php'
    else:
      self.login_url="http://www.newsmth.net/bbslogin.php"
      self.logout_url="http://www.newsmth.net/bbslogout.php"
      
    return

  def set_proxy(self, proxy):
    self.proxy=proxy
    self.cookie=cookielib.CookieJar()
    if self.proxy:
      self.opener=urllib2.build_opener(urllib2.ProxyHandler({'http':proxy}),
          urllib2.HTTPCookieProcessor(self.cookie))
    else:
      self.opener=urllib2.build_opener(urllib2.HTTPCookieProcessor(self.cookie))
    return
  
  def login(self):
    try:
      post_data=urllib.urlencode([('id',self.user),('passwd', self.password)])
      post_data+="&mode=1&CookieDate=0"
      req=urllib2.Request(self.login_url)
      fd=self.opener.open(req, post_data)
      print fd.read()
      is_success = False
      for c in self.cookie:
        print "%s: %s" %(self.user, str(c))
        if c.name == "UTMPUSERID" and c.value.lower() == self.user.lower():
          is_success = True
    except Exception, e:
      print >>sys.stderr, '%s login Error! Reason: %s' %(self.user, str(e))
      return False

    return is_success

  def is_valid(self):
    for c in self.cookie:
      if c.name == "UTMPUSERID" and c.value.lower() == self.user.lower() \
        and not c.is_expired() :
        return True
    return False

  def noop(self, noop_url = "http://www.newsmth.net/bbscon.php?bid=1094&id=2839653"):
    try:
      fd = self.opener.open(noop_url)
      fd.read()
    except Exception, e:
     print >>sys.stderr, "Error: %s" %(str(e))
     return False
    return True
    
  def queryScore(self, user):
    query_url = "http://www.newsmth.net/bbsqry.php?userid=%s" %user
    try:
      fd = self.opener.open(query_url)
      buf = fd.read().decode("gbk")
      pattern = u'\u79ef\u5206\s*:\s*\[(\d+)\]'
      result = re.search(pattern, buf)
      if result:
        score = long(result.group(1))
        return score
    except Exception, e:
      print >>sys.stderr, "Error in getting the score of %s...%s" %(user, str(e))
    return None

  def logout(self):
    try:
      headers = {}
      headers["Referer"] = "http://www.newsmth.net/bbsleft.php"
      headers["Accept-Language"] = "zh-cn"
      req=urllib2.Request(self.logout_url, None, headers)
      fd=self.opener.open(req)
      print fd.read()
      print fd.headers
    except Exception, e:
      print >>sys.stderr, 'Logout Error! %s, Reason: %s' %(self.user, str(e))
      return False
    return True

  def modify_article(board, article_id, need_check=False):
##    article_id=str(article_id)
##    url='http://www.2.newsmth.net/bbsedit.php?board='+board+'&id='+article_id+'&ftype=0'##&do'
##    print '....to modify:', url
##    
##    try:
##      ##first: we try to read the article to find the original author.
##      ##original_url="http://www.2.newsmth.net/bbscon.php?bid=%d&id=%d" %(board_id, article_id)
##      fd=opener.open(url)
##      data=fd.read()
##      data=data.decode('gbk','replace')
##      parser=BeautifulSoup.BeautifulSoup(data)
##
##      title=parser.first("input", {"name":"title"})
##      article_title=title.get("value")
##      article_title=article_title.encode("gbk","replace")    
##      if need_check and re.match(TITLE_PATTERN, article_title):
##        print "Already Handled...", article_title
##        return True
##      
##      content=parser.first("textarea")
##      article=content.string
##      m=re.search(ARTICLE_AUTHOR_PATTERN, article)
##      if m is None :
##        print "Error: Not zz! ", article.encode("gbk","replace")
##        return True
##
##      original_author=m.group("author")
##      original_author=original_author.encode("gbk", "replace")
##      article=article.encode("gbk", "replace")
##      print "Original: ", original_author
##      print "title: ", article_title
##      post_data=[('title','[%s]%s' %(original_author, article_title)),('text',article)]
##      post_data=urllib.urlencode(post_data)
##      req=urllib2.Request(url+"&do")
##      fd=opener.open(req, post_data)
##      fd.read()
##    except Exception, e:
##      print 'error in posting data', e
##      failed_articles.write("%s,%s\n" %(board, article_id))
##      return False
##    return True
    return

  def post_article(self, board, title, content, qmd=1, reid=0): ##reid!=0 means replying ,else new thread.
    url="http://www.newsmth.net/"
    if self.is2site:
      url="http://www.2.newsmth.net/"
    url+="bbssnd.php?board="+board+"&reid="+str(reid)
    ##following paramters
    ##title
    ##attachname
    ##signature: 0/not use/; i>1(selected; -1/random/
    ##mailback
    ##havemath
    ##text
    try:
      if type(title)==unicode:
        title=title.encode("gbk", "replace")
      if type(content)==unicode:
        content=content.encode("gbk", "replace")
      
      post_data=[('title',title),('attachname',''), ('signature', "%d" %qmd),
            ('text',content)
           ]
      post_data=urllib.urlencode(post_data)

      req=urllib2.Request(url)
      fd=self.opener.open(req, post_data)
      s=fd.read()
      ##print s##.decode("utf-8")
    except Exception, e:
      print "Error in posting an article", e
      return False
    return True
