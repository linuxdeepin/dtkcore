#!/usr/bin/env python3

import fnmatch
import os
import argparse
import re
import datetime


license_header = '''/*\n'''
license_empty = ''' *\n'''
license_copyright_prifix = ''' * Copyright (C) {0}\n'''
license_copyright_indent = ''' *               {0}\n'''
license_author_prifix = ''' * Author:     {0}\n'''
license_person_indent = ''' *             {0}\n'''
license_maintainer_prifix = ''' * Maintainer: {0}\n'''
license_tail = ''' */\n\n'''
default_license_body = ''' * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
'''


def isInt(value):
    try:
        int(value)
        return True
    except ValueError:
        return False


class Copyright:
    def __init__(self, line):
        self.year_start = datetime.datetime.now().year
        self.year_end = datetime.datetime.now().year

        if "copyright" in line.lower():
            line = line[re.search("copyright", line, re.IGNORECASE).end():]

        copyright_sysbol = re.search("\(.\)", line, re.IGNORECASE)
        if copyright_sysbol is not None:
            copyright_brace_end_pos = copyright_sysbol.end()
            line = line[copyright_brace_end_pos:]
            pass

        if re.search("[0-9]+", line) is not None:
            year_start_pos = re.search("[0-9]+", line).start()
            year_start_end_pos = re.search("[0-9]+", line).end()
            self.year_start = int(line[year_start_pos: year_start_end_pos])
            line = line[year_start_end_pos:]

        if re.search("[0-9]+", line) is not None:
            year_end_pos = re.search("[0-9]+", line).start()
            year_end_end_pos = re.search("[0-9]+", line).end()
            if re.search("~", line) is not None:
                year_spilt = re.search("~", line).start()
                if (year_spilt < year_end_pos):
                    self.year_end = int(line[year_end_pos: year_end_end_pos])
                    line = line[year_end_end_pos:]

        self.name = line.strip()

    def string(self):
        return "{0} ~ {1} {2}".format(
            self.year_start,
            self.year_end,
            self.name)


class Person:
    def __init__(self, line):
        if re.search(":", line, re.IGNORECASE) is not None:
            split_end = re.search(":", line).end()
            line = line[split_end:]

        self.name = line.strip()


def filter_files(file_list, ext_list):
    for file in file_list:
        for ext in ext_list:
            if fnmatch.fnmatch(file, ext):
                yield file
                break


class Source:
    def __init__(self, filename):
        self.body = []
        self.copyrights = {}
        self.authors = {}
        self.maintainers = {}
        self.filename = filename
        self.is_deepin_copyright = False
        self.update_license(filename)

    def update_license(self, filename):
        find_license_start = False
        find_license_end = False

        parse_copyright_start = False
        parse_author_start = False
        parse_maintainer_start = False

        with open(filename, "r") as f:
            print("process:", filename)
            for line in f:
                if line.strip().startswith("/*") and (0 == len(self.body)):
                    find_license_start = True

                if (0 != len(self.body)) or (0 != len(line.strip())):
                    self.body.append(line)

                if "*/" in line.strip() and find_license_start:
                    if not find_license_end:
                        # print("clean body")
                        self.body = []
                    find_license_end = True

                if find_license_start and not find_license_end:
                    if re.search("(\*)+", line):
                        # remove *****
                        line = line[re.search("(\*)+", line).end():]

                    if "copyright" in line.lower():
                        parse_copyright_start = True
                        parse_author_start = False
                        parse_maintainer_start = False
                    if "author" in line.lower():
                        parse_copyright_start = False
                        parse_author_start = True
                        parse_maintainer_start = False
                    if "maintainer" in line.lower():
                        parse_copyright_start = False
                        parse_author_start = False
                        parse_maintainer_start = True
                    if 0 == len(line.strip()):
                        parse_copyright_start = False
                        parse_author_start = False
                        parse_maintainer_start = False

                    if parse_copyright_start:
                        cr = Copyright(line)
                        self.copyrights[cr.name] = cr

                    if parse_author_start:
                        p = Person(line)
                        self.authors[p.name] = p

                    if parse_maintainer_start:
                        p = Person(line)
                        self.maintainers[p.name] = p

    def fix_deepin(self):
        new_cr = {}
        self.is_deepin_copyright = False
        for k, cr in self.copyrights.items():
            if "deepin" in cr.name.lower():
                cr.name = "Deepin Technology Co., Ltd."
                self.is_deepin_copyright = True
            new_cr[cr.name] = cr
        self.copyrights = new_cr

    def dump(self):
        for cr in self.copyrights:
            print(cr.year_start, cr.year_end, cr.name)
        for a in self.authors:
            print(a.name)
        for a in self.maintainers:
            print(a.name)
        print("body size:", len(self.body))

    def save(self):
        wf = open(self.filename, 'w')
        wf.write(license_header)

        if len(self.copyrights):
            write_first = False
            for k, cr in self.copyrights.items():
                if not write_first:
                    wf.write(license_copyright_prifix.format(cr.string()))
                    write_first = True
                else:
                    wf.write(license_copyright_indent.format(cr.string()))
            wf.write(license_empty)

        if len(self.authors):
            write_first = False
            for k, author in self.authors.items():
                if not write_first:
                    wf.write(license_author_prifix.format(author.name))
                    write_first = True
                else:
                    wf.write(license_person_indent.format(author.name))
            wf.write(license_empty)

        if len(self.maintainers):
            write_first = False
            for k, maintainer in self.maintainers.items():
                if not write_first:
                    wf.write(license_maintainer_prifix.format(maintainer.name))
                    write_first = True
                else:
                    wf.write(license_person_indent.format(maintainer.name))
            wf.write(license_empty)

        wf.write(default_license_body)
        wf.write(license_tail)

        for l in self.body:
            wf.write(l)
        wf.close()


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('--authors', help='''authors split by ":". For example:\n
        "Iceyer <me@iceyer.net>:Li He <lihe@deepin.com>"''')
    parser.add_argument('--maintainers', help='''maintainers split by ":". For example: \n
        "Iceyer <me@iceyer.net>:Li He <lihe@deepin.com>"''')
    parser.add_argument('--copyrights', help='''copyrights split by ":". For example: \n
        "2011 ~ 2017 Deepin Technology Co., Ltd.:2011 ~ 2017 Li He"''')
    parser.add_argument('--fix-deepin',  dest='fixdeepin',
                        action='store_const', const=True, default=False, help='''
                        fix deein copyrights''')
    parser.add_argument('dir',  help='''
                        source code dir''')

    args = parser.parse_args()
    return args


def match_files(dir):
    EXTENSIONS = "*.cpp", "*.c", "*.h", "*.go"
    matches = []
    for root, dirnames, filenames in os.walk(dir):
        for filename in filter_files(filenames, EXTENSIONS):
            matches.append(os.path.join(root, filename))
    return matches


def test():
    cr = Copyright("Copyright (C)  Deepin Technology Co., Ltd.")
    print(cr.year_start, cr.year_end, cr.name)
    cr = Copyright("Copyright (C) 2007 Deepin Technology Co., Ltd.")
    print(cr.year_start, cr.year_end, cr.name)
    cr = Copyright("Copyright (C) 2007 ~ 2016 Deepin Technology Co., Ltd.")
    print(cr.year_start, cr.year_end, cr.name)

    p = Person("Author:     Wang Yong <wangyong@deepin.com>")
    print(p.name)
    p = Person("    Wang Yong <wangyong@deepin.com>")
    print(p.name)


def get_authors(args):
    authors = []
    if args.authors is not None:
        for a in args.authors.split(":"):
            authors.append(Person(a))
    return authors


def get_copyrights(args):
    copyrights = []
    if args.copyrights is not None:
        for a in args.copyrights.split(":"):
            copyrights.append(Copyright(a))
    return copyrights


def get_maintainers(args):
    maintainers = []
    if args.maintainers is not None:
        for a in args.maintainers.split(":"):
            maintainers.append(Copyright(a))
    return maintainers


def main():
    args = parse_args()
    authors = get_authors(args)
    copyrights = get_copyrights(args)
    maintainers = get_maintainers(args)
    fixdeepin = args.fixdeepin
    files = match_files(args.dir)

    for filename in files:
        s = Source(filename)
        # s.dump()
        if fixdeepin:
            s.fix_deepin()
        if not s.is_deepin_copyright and 0 != len(s.copyrights):
            continue

        if 0 == len(s.copyrights):
            for cr in copyrights:
                s.copyrights[cr.name] = cr

        for p in authors:
            s.authors[p.name] = p

        for p in maintainers:
            s.maintainers[p.name] = p

        s.save()
        # s.dump()


if __name__ == "__main__":
    main()
