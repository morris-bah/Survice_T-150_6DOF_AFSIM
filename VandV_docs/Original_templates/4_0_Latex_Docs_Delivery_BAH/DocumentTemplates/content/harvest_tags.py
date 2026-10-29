from glob import glob

tags = {'UAM': ('%[UAM]\n', '%[/UAM]\n'),
        'UAM2': ('%[UAM2]\n', '%[/UAM2]\n'),
        'ADD': ('%[ADD]\n', '%[/ADD]\n'),
        'ICD': ('%[ICD]\n', '%[/ICD]\n'),
        'TDD': ('%[TDD]\n', '%[/TDD]\n'),
        'BOTH': ('%[BOTH]\n', '%[/BOTH]\n')}

def capture_tag(string: str, document: str) -> list[str]:
    '''
    Return a list of all substrings in 'string' that are delimited by
    the 'document' tag.
    '''
    open_tag, close_tag = tags[document]
    kept = []
    while string:
        discard, tag, string = string.partition(open_tag)
        keep, tag, string = string.partition(close_tag)
        kept.append(keep)
    return kept

def convert_tags(string: str, to_doc: str) -> str:
    '''
    Turn all 'BOTH' tags in 'string' into 'to_doc' tags.
    '''
    string = string.replace(tags['BOTH'][0], tags[to_doc][0])
    string = string.replace(tags['BOTH'][1], tags[to_doc][1])
    return string

def harvest_tags(source_filename: str, document: str) -> None:
    '''
    Create a file that contains all the portions of 'source_filename' that are
    delimited by 'document' or by 'BOTH'.  This file will have the filename
    "'document''output_filename_suffix'.tex"
    '''
    with open(source_filename) as file_in:
        file_text = file_in.read()
    file_text = convert_tags(file_text, document)
    portions_to_keep = capture_tag(file_text, document)
    output_filename = f'pySubfiles\\{document}-{source_filename}'
    write_success = False
    while not write_success:
        try:
            with open(output_filename, 'w') as file_out:
                file_out.write('\n'.join(portions_to_keep))
            write_success = True
        except FileNotFoundError:
            mkdir('pySubfiles')
    print(f'Created {output_filename} from the {document} and BOTH portions '
          f'of {source_filename}.')


if __name__ == '__main__':
    
    for component_filename in glob(f'Component*tex'):
        for document in ('ADD', 'UAM'):
            harvest_tags(component_filename, document)
    for document in ('ADD','UAM','ICD','TDD','UAM2'):
        harvest_tags('Introduction.tex', document)
        harvest_tags('SystemDescription.tex', document)
