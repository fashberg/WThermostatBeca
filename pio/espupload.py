#!/usr/bin/python
#
# espupload by Theo Arends - 20170930
# Modified for WTHermostat by Folke Ashberg - 202010
#
# Uploads binary file to OTA server
#
# Execute: espupload -u <Host_IP_address> -f <sketch.bin>
#
# Needs pycurl
#   - pip install pycurl
#   - Windows: Install WHL-File (pip install <downloaded-whl>) from https://www.lfd.uci.edu/~gohlke/pythonlibs/#pycurl
#       Which must match your python installation


import sys
import os
import optparse
import logging
from io import StringIO
from html.parser import HTMLParser
import pycurl

class MLStripper(HTMLParser):
  def __init__(self):
      super().__init__()
      self.reset()
      self.strict = False
      self.convert_charrefs= True
      self.text = StringIO()
  def handle_data(self, d):
      self.text.write(d)
  def get_data(self):
      return self.text.getvalue()

def upload(host, filename):

  url = 'http://%s/firmware' % (host)
  c = pycurl.Curl()
  c.setopt(c.URL, url)
  # The "Expect:" is there to suppress "Expect: 100-continue" behaviour that is
  # the default in libcurl when posting large bodies (and fails on lighttpd).
  c.setopt(c.HTTPHEADER, ["Expect:"])
  c.setopt(c.HTTPPOST, [('file', (c.FORM_FILE, filename, )), ])
  html=c.perform_rs()
  c.close()
  s = MLStripper()
  s.feed(html)
  print( s.get_data().replace("\n\n", "\n").replace("\n\n", "\n"))

def parser():
  parser = optparse.OptionParser(
    usage = "%prog [options]",
    description = "Upload image to over the air Host server for the esp8266 module with OTA support."
  )

  # destination ip and port
  group = optparse.OptionGroup(parser, "Destination")
  group.add_option("-u", "--host",
    dest = "host",
    action = "store",
    help = "Host Address",
    default = None
  )
  parser.add_option_group(group)

  # image
  group = optparse.OptionGroup(parser, "Image")
  group.add_option("-f", "--file",
    dest = "image",
    help = "Image file.",
    metavar = "FILE",
    default = None
  )
  parser.add_option_group(group)

  # output group
  group = optparse.OptionGroup(parser, "Output")
  group.add_option("-d", "--debug",
    dest = "debug",
    help = "Show debug output. And override loglevel with debug.",
    action = "store_true",
    default = False
  )
  parser.add_option_group(group)

  (options, args) = parser.parse_args()

  return options
# end parser

def main(args):
  # get options
  options = parser()

  # adapt log level
  loglevel = logging.WARNING
  if (options.debug):
    loglevel = logging.DEBUG
  # end if

  # logging
  logging.basicConfig(level = loglevel, format = '%(asctime)-8s [%(levelname)s]: %(message)s', datefmt = '%H:%M:%S')

  logging.debug("Options: %s", str(options))

  if (not options.host or not options.image):
    logging.critical("Not enough arguments.")

    return 1
  # end if

  if not os.path.exists(options.image):
    logging.critical('Sorry: the file %s does not exist', options.image)

    return 1
  # end if

  upload(options.host, options.image)
# end main

if __name__ == '__main__':
  sys.exit(main(sys.argv))
# end if