#!/usr/bin/env python3

from os import listdir, path
from datetime import datetime


def report(output, desc, dikt_has, options, doc, option_count, all_dikts=False):
    # header
    output.write('## {} Options\n\n'.format(desc))
    difopt = 0
    for option in options:
        if option in option_count:
            difopt += 1
    if difopt == 0:
        output.write('A total of {} {} different options are recognised by Hunspell. None of these options are used'.format(
        len(options), desc.lower()))
    elif difopt == len(options):
        output.write('A total of {} {} different options are recognised by Hunspell. All of these options are used'.format(
        len(options), desc.lower()))
    elif difopt == 1:
        output.write('A total of {} {} different options are recognised by Hunspell. Of these, only 1 option is used'.format(
        len(options), desc.lower()))
    else:
        output.write('A total of {} {} different options are recognised by Hunspell. Of these, only {} different options are used'.format(
        len(options), desc.lower(), difopt))
    if len(dikt_has) == 0:
        output.write('\n\n')
    elif len(dikt_has) == 1:
        if len(dikt_has) == len(doc):
            output.write(' in all 1 dictionary.\n\n')
        else:
            output.write(' in only 1 dictionary.\n\n')
    else:
        if len(dikt_has) == len(doc):
            output.write(' in all {} dictionaries.\n\n'.format(len(dikt_has)))
        else:
            output.write(' in only {} dictionaries.\n\n'.format(len(dikt_has)))
        
    output.write('| {} vs. Dictionary'.format(desc))
    if all_dikts:
        for dikt in sorted(doc):
            output.write(' | {}'.format(dikt.replace('_', '\_')))
    else:
        for dikt in sorted(dikt_has):
            output.write(' | {}'.format(dikt.replace('_', '\_')))
    output.write(' |\n')

    # format
    output.write('|---')
    if all_dikts:
        for dikt in sorted(doc):
            output.write('|--:')
    else:
        for dikt in sorted(dikt_has):
            output.write('|--:')
    output.write('|\n')

    # content
    for option in options:
        output.write('| {}'.format(option))
        if all_dikts:
            for dikt in sorted(doc):
                oc = doc[dikt]
                if option in oc:
                    output.write(' | {}'.format(oc[option]))
                else:
                    output.write(' |')
        else:
            for dikt in sorted(dikt_has):
                oc = doc[dikt]
                if option in oc:
                    output.write(' | {}'.format(oc[option]))
                else:
                    output.write(' |')
        output.write(' |\n')
    output.write('\n\n')

# followings list are manually obtained from $ man -K 5 hunspell
options_general = ('SET', 'FLAG', 'COMPLEXPREFIXES',
                   'LANG', 'IGNORE', 'AF', 'AM', )
options_suggest = ('KEY', 'TRY', 'NOSUGGEST', 'MAXCPDSUGS', 'MAXNGRAMSUGS', 'MAXDIFF',
                   'ONLYMAXDIFF', 'NOSPLITSUGS', 'SUGSWITHDOTS', 'REP', 'MAP', 'PHONE', 'WARN', 'FORBIDWARN', )
options_compounding = ('BREAK', 'COMPOUNDRULE', 'COMPOUNDMIN', 'COMPOUNDFLAG', 'COMPOUNDBEGIN', 'COMPOUNDLAST', 'COMPOUNDMIDDLE', 'ONLYINCOMPOUND', 'COMPOUNDPERMITFLAG', 'COMPOUNDFORBIDFLAG', 'COMPOUNDMORESUFFIXES', 'COMPOUNDROOT',
                       'COMPOUNDWORDMAX', 'CHECKCOMPOUNDDUP', 'CHECKCOMPOUNDREP', 'CHECKCOMPOUNDCASE', 'CHECKCOMPOUNDTRIPLE', 'SIMPLIFIEDTRIPLE', 'CHECKCOMPOUNDPATTERN', 'FORCEUCASE', 'COMPOUNDSYLLABLE', 'SYLLABLENUM', )  # 'COMPOUND',
options_affix = ('PFX', 'SFX', 'CIRCUMFIX', 'FORBIDDENWORD', 'FULLSTRIP', 'KEEPCASE',
                 'ICONV', 'OCONV', 'NEEDAFFIX', 'SUBSTANDARD', 'WORDCHARS', 'CHECKSHARPS', )
options_deprecated = ('LEMMA_PRESENT', 'PSEUDOROOT', )

# self-check
for o in options_general:
    if o in options_suggest:
        print('ERROR: Overlap general and sugest')
        exit(1)
    if o in options_compounding:
        print('ERROR: Overlap general and compounding')
        exit(1)
    if o in options_affix:
        print('ERROR: Overlap general and affix')
        exit(1)
    if o in options_deprecated:
        print('ERROR: Overlap general and deprecated')
        exit(1)
for o in options_suggest:
    if o in options_compounding:
        print('ERROR: Overlap suggest and compounding')
        exit(1)
    if o in options_affix:
        print('ERROR: Overlap suggest and affix')
        exit(1)
    if o in options_deprecated:
        print('ERROR: Overlap sugges and deprecated')
        exit(1)
for o in options_compounding:
    if o in options_affix:
        print('ERROR: Overlap compounding and affix')
        exit(1)
    if o in options_deprecated:
        print('ERROR: Overlap compounding and deprecated')
        exit(1)
for o in options_affix:
    if o in options_deprecated:
        print('ERROR: Overlap affix and deprecated')
        exit(1)

options_found = []
options_undocumented = []
option_count = {}  # option / count

doc = {}  # dictionary / option / count
options = []
dikt_has_general = []
dikt_has_suggest = []
dikt_has_compounding = []
dikt_has_affix = []
dikt_has_deprecated = []
dikt_has_undocumented = []

directory = 'usr/share/hunspell/'
for filename in listdir(directory):
    filepath = directory + filename
    if filename.endswith('.aff') and path.islink(filepath):
        print('XX', filename)
    if filename.endswith('.dic') and not path.islink(filepath):
        print('YY', filename)
    if not filename.endswith('.aff') or path.islink(filepath) or filename in ('kk_KZ.aff', ):  #FIXME kk_KZ.aff has invalid first character
        continue
    input = None
    print(filename)
    if filename in ('de_AT_frami.aff', 'de_CH_frami.aff', 'de_DE_frami.aff', 'de_DE.aff', 'en_US.aff', 'pt_BR.aff', 'sl_SI.aff', 'th_TH.aff', 'ru_RU.aff', 'nn_NO.aff', 'an_ES.aff', 'af_ZA.aff', 'el_GR.aff', 'bg_BG.aff', 'de_CH.aff', 'it_IT.aff', 'hu_HU.aff', 'pl_PL.aff', 'cs_CZ.aff', 'eu.aff', 'lt_LT.aff', 'nb_NO.aff', 'oc_FR.aff', 'bs_BA.aff', 'de_AT.aff', ):
        input = open(filepath, 'r', encoding='ISO-8859-1')
    else:
        input = open(filepath, 'r')
    dikt = filename.replace('.aff', '')
    doc[dikt] = {}
    oc = doc[dikt]
    for line in input:
        if dikt == 'kk_KZ':
            line.replace('﻿', '')
            print(line)
        line = line.strip()
        if line == '' or line.startswith('#'):
            continue
        while '  ' in line:  # TODO
            line = line.replace('  ', ' ')
        while '\t' in line:  # TODO report?
            line = line.replace('\t', ' ')
        br = line.split(' ')
        option = br[0]
#            print(option, oc[option])
        if option not in options_found:
            options_found.append(option)
        if option in oc:
            oc[option] += 1
        else:
            oc[option] = 1
            if option in options_general:
                if dikt not in dikt_has_general:
                    dikt_has_general.append(dikt)
            elif option in options_suggest:
                if dikt not in dikt_has_suggest:
                    dikt_has_suggest.append(dikt)
            elif option in options_compounding:
                if dikt not in dikt_has_compounding:
                    dikt_has_compounding.append(dikt)
            elif option in options_affix:
                if dikt not in dikt_has_affix:
                    dikt_has_affix.append(dikt)
            elif option in options_deprecated:
                if dikt not in dikt_has_deprecated:
                    dikt_has_deprecated.append(dikt)
            else:
                if dikt not in dikt_has_undocumented:
                    dikt_has_undocumented.append(dikt)
                if option not in options_undocumented:
                    options_undocumented.append(option)
        if option in option_count:
            option_count[option] += 1
        else:
            option_count[option] = 1

output = open('option-usage.md', 'w')
output.write('# Hunspell Option Usage per Dictionary\n\n')
output.write('This page has been generated at {}. Do not edit this page manually.\n\n'.format(datetime.now().strftime('%Y-%m-%d %H:%M:%S').replace(' ' , ' at ')))

print('dictionaries found', len(doc))
print('options found', len(options_found))

report(output, 'General', dikt_has_general, options_general, doc, option_count, all_dikts=True)
report(output, 'Suggest', dikt_has_suggest, options_suggest, doc, option_count)
report(output, 'Compounding', dikt_has_compounding, options_compounding, doc, option_count)
report(output, 'Affix', dikt_has_affix, options_affix, doc, option_count)
report(output, 'Deprecated', dikt_has_deprecated, options_deprecated, doc, option_count)
report(output, 'Undocumented', dikt_has_undocumented, options_undocumented, doc, option_count)
