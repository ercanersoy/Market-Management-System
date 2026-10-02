#!/usr/bin/env python3
"""Convert a README_*.md file into a plain DOS text file.

Usage: md2txt.py INPUT.md OUTPUT.TXT ENCODING

The output has CR/LF line endings, lines of at most 78 characters and is
encoded with ENCODING (for example ascii or cp857), so it can be read with
TYPE or MORE in MS-DOS.
"""

import re
import sys
import textwrap

WIDTH = 78


def inline(text):
    """Remove Markdown inline markup."""
    # links: keep the text, add the address for web links
    def link(m):
        label, url = m.group(1), m.group(2)
        if re.match(r'[a-z]+://', url):
            return '%s (%s)' % (label, url)
        return label
    text = re.sub(r'\[([^\]]*)\]\(([^)]*)\)', link, text)
    text = re.sub(r'\*\*([^*]+)\*\*', r'\1', text)
    text = text.replace('`', '')
    return text


def wrap(text, first='', rest=''):
    return textwrap.wrap(inline(text), WIDTH, initial_indent=first,
                         subsequent_indent=rest, break_long_words=False,
                         break_on_hyphens=False) or [first.rstrip()]


def table(rows):
    cells = [[inline(c.strip()) for c in r.strip().strip('|').split('|')]
             for r in rows]
    head, body = cells[0], cells[2:]
    cols = len(head)
    fixed = [max(len(r[i]) for r in [head] + body) for i in range(cols - 1)]
    last = WIDTH - sum(fixed) - 2 * (cols - 1)
    out = []
    if last >= 24:
        def line(r):
            first = ''.join(c.ljust(w + 2) for c, w in zip(r, fixed))
            parts = textwrap.wrap(r[-1], last, break_long_words=False,
                                  break_on_hyphens=False) or ['']
            res = [(first + parts[0]).rstrip()]
            res += [(' ' * len(first) + p).rstrip() for p in parts[1:]]
            return res
        out += line(head)
        out.append('  '.join('-' * w for w in fixed + [min(last, max(
            len(r[-1]) for r in [head] + body))]))
        for r in body:
            out += line(r)
    else:
        # too wide: one block per row
        for r in body:
            out.append(r[0])
            for h, c in zip(head[1:], r[1:]):
                out += wrap('%s: %s' % (h, c), '    ', '    ')
    return out


def convert(src):
    lines = src.splitlines()
    out = []
    i = 0
    while i < len(lines):
        s = lines[i]
        if s.startswith('```'):
            i += 1
            while not lines[i].startswith('```'):
                out.append(('    ' + lines[i]).rstrip())
                i += 1
            out.append('')
            i += 1
        elif s.startswith('#'):
            level = len(s) - len(s.lstrip('#'))
            title = inline(s.lstrip('#').strip())
            if out and out[-1] != '':
                out.append('')
            if level == 1:
                out += ['=' * len(title), title, '=' * len(title)]
            elif level == 2:
                out += [title, '-' * len(title)]
            else:
                out += [title, '~' * len(title)]
            out.append('')
            i += 1
        elif s.startswith('|'):
            rows = []
            while i < len(lines) and lines[i].startswith('|'):
                rows.append(lines[i])
                i += 1
            out += table(rows)
            out.append('')
        elif re.match(r'\s*\* ', s):
            while i < len(lines) and re.match(r'\s*\* ', lines[i]):
                ind = len(lines[i]) - len(lines[i].lstrip())
                text = lines[i].strip()[2:]
                i += 1
                while (i < len(lines) and lines[i].strip() and
                       not re.match(r'\s*\* ', lines[i])):
                    text += ' ' + lines[i].strip()
                    i += 1
                pad = ' ' * ind
                out += wrap(text, pad + '  * ', pad + '    ')
            out.append('')
        elif not s.strip():
            i += 1
        else:
            text = s.strip()
            i += 1
            while (i < len(lines) and lines[i].strip() and
                   not re.match(r'(\s*\* |#|\||```)', lines[i])):
                text += ' ' + lines[i].strip()
                i += 1
            out += wrap(text)
            out.append('')
    while out and out[-1] == '':
        out.pop()
    for line in out:
        assert len(line) <= WIDTH, line
    return '\r\n'.join(out) + '\r\n'


def main():
    src, dst, enc = sys.argv[1:4]
    with open(src, encoding='utf-8') as f:
        text = convert(f.read())
    with open(dst, 'wb') as f:
        f.write(text.encode(enc))


if __name__ == '__main__':
    main()
