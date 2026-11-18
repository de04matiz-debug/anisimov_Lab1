"""Integration checks. Run after building: python tests/lab3/check_lab3.py <exe>"""
from pathlib import Path
import random
import re
import subprocess
import sys
import tempfile


def data(pipes=(), stations=(), edges=None):
    lines = [str(len(pipes))]
    for pipe in pipes:
        lines.extend(map(str, pipe))
    lines.append(str(len(stations)))
    for station in stations:
        lines.extend(map(str, station))
    if edges is not None:
        lines.extend(('NETWORK_V1', str(len(edges))))
        for edge in edges:
            lines.extend(map(str, edge))
    return '\n'.join(lines) + '\n'


def graph(ids, links, spare=()):
    pipes = [(i + 1, f'Pipe {i + 1}', 1.25, 500, 0) for i in range(len(links))]
    pipes.extend(spare)
    stations = [(i, f'Station {i}', 4, 2, 1) for i in ids]
    edges = [(i + 1, start, end) for i, (start, end) in enumerate(links)]
    return data(pipes, stations, edges)


def saved(content):
    lines = iter(content.decode('utf-8').splitlines())
    pipes, stations, edges = {}, {}, {}
    for _ in range(int(next(lines))):
        pipe_id = int(next(lines))
        pipes[pipe_id] = (next(lines), float(next(lines)), int(next(lines)), int(next(lines)))
    for _ in range(int(next(lines))):
        station_id = int(next(lines))
        stations[station_id] = (next(lines), int(next(lines)), int(next(lines)), int(next(lines)))
    assert next(lines) == 'NETWORK_V1'
    for _ in range(int(next(lines))):
        pipe_id = int(next(lines))
        assert pipe_id not in edges
        edges[pipe_id] = (int(next(lines)), int(next(lines)))
        assert pipe_id in pipes and all(i in stations for i in edges[pipe_id])
    assert list(lines) == []
    return pipes, stations, edges


def valid_order(output, ids, links):
    matches = re.findall(r'Топологический порядок КС:([^\r\n]*)', output)
    assert len(matches) == 1, output
    order = list(map(int, matches[0].split()))
    assert len(order) == len(ids) and set(order) == set(ids), order
    position = {station_id: i for i, station_id in enumerate(order)}
    assert all(position[a] < position[b] for a, b in links), (order, links)
    assert 'в сети есть цикл' not in output


def has_cycle(ids, links):
    # Independent DFS oracle for the small random graphs.
    adjacent = {i: [] for i in ids}
    for start, end in links:
        adjacent[start].append(end)
    color = {i: 0 for i in ids}

    def visit(station_id):
        color[station_id] = 1
        for next_id in adjacent[station_id]:
            if color[next_id] == 1 or (color[next_id] == 0 and visit(next_id)):
                return True
        color[station_id] = 2
        return False

    return any(color[i] == 0 and visit(i) for i in ids)


class Checks:
    def __init__(self, exe):
        self.exe = exe.resolve()
        self.count = 0

    def run(self, name, typed, files=None, contains=(), excludes=()):
        try:
            with tempfile.TemporaryDirectory(prefix='lab3_check_') as temp:
                cwd = Path(temp)
                for filename, content in (files or {}).items():
                    (cwd / filename).write_text(content, encoding='utf-8', newline='\n')
                result = subprocess.run([self.exe], input=typed.encode('utf-8'), cwd=cwd,
                                        capture_output=True, timeout=15)
                assert result.returncode == 0, (result.returncode, result.stderr)
                assert result.stderr == b'', result.stderr
                log = (cwd / 'log.txt').read_bytes().replace(b'\r\n', b'\n')
                assert log == typed.encode('utf-8'), ('log differs from input', log, typed)
                output = result.stdout.decode('utf-8')
                for text in contains:
                    assert text in output, (text, output)
                for text in excludes:
                    assert text not in output, (text, output)
                snapshots = {p.name: p.read_bytes() for p in cwd.iterdir() if p.is_file()}
            self.count += 1
            return output, snapshots
        except Exception as error:
            raise AssertionError(f'{name}: {error}') from error

    def load(self, name, fixture, commands='11\n12\n0\n', **kwargs):
        return self.run(name, '7\nfixture.txt\n' + commands, {'fixture.txt': fixture}, **kwargs)


def main(exe):
    checks = Checks(exe)
    stations = [(i, f'КС {i}', 4, 2, 1) for i in range(1, 5)]
    pipes = [(1, 'Труба 1', 1.25, 500, 0), (2, 'Труба 2', 2.5, 500, 0),
             (3, 'Труба 3', 3, 700, 0), (4, 'Труба 4', 4, 1000, 0),
             (5, 'Труба 5', 5, 1400, 0)]
    base = data(pipes, stations, [])

    checks.run('empty', '11\n12\n10\n13\n0\n', contains=('КС не добавлены', 'хотя бы две КС', 'нет соединений'))
    checks.load('one station', data([], stations[:1], []), '10\n12\n0\n', contains=('хотя бы две КС',))
    for diameter, pipe_id in ((500, 1), (700, 3), (1000, 4), (1400, 5)):
        output, files = checks.load(f'diameter {diameter}', base,
                                   f'10\n1\n2\n{diameter}\n6\nout.txt\n12\n0\n',
                                   contains=(f'труба ID={pipe_id}.',), excludes=('Создайте трубу',))
        actual_pipes, actual_stations, edges = saved(files['out.txt'])
        assert edges == {pipe_id: (1, 2)} and len(actual_pipes) == 5 and len(actual_stations) == 4
        valid_order(output, range(1, 5), [(1, 2)])

    fixture = data([(91, 'A', 1, 500, 0), (4, 'B', 1, 500, 0), (27, 'C', 1, 500, 0)], stations, [])
    checks.load('smallest free ID', fixture, '10\n1\n2\n500\n0\n', contains=('труба ID=4.',))
    fixture = graph([1, 2, 3], [(1, 2)], [(2, 'Spare', 1, 500, 0)])
    checks.load('occupied pipe excluded', fixture, '10\n2\n3\n500\n0\n', contains=('труба ID=2.',), excludes=('Создайте трубу',))
    fixture = data([(6, 'Repair', 1, 500, 1)], stations, [])
    checks.load('repair preserves physical topology', fixture, '10\n1\n2\n500\n12\n0\n', contains=('труба ID=6.',))

    output, files = checks.load('automatic creation', data([], stations, []),
                               '10\n1\n2\n1000\n\n  Новая труба  \nx\n0\n2.75\n2\n0\n6\nданные сети.txt\n0\n',
                               contains=('Создайте трубу', 'Создана труба, ID: 1', 'труба ID=1.'))
    actual_pipes, _, edges = saved(files['данные сети.txt'])
    assert actual_pipes[1] == ('  Новая труба  ', 2.75, 1000, 0) and edges == {1: (1, 2)}
    assert output.count('Диаметр трубы (500, 700, 1000, 1400 мм):') == 1
    assert 'Диаметр (500, 700, 1000, 1400 мм):' not in output

    _, files = checks.load('wrong diameter creates new pipe', data(pipes[:1], stations, []),
                          '10\n1\n2\n700\nНовая\n1\n0\n6\nout.txt\n0\n')
    actual_pipes, _, edges = saved(files['out.txt'])
    assert actual_pipes[1][2] == 500 and actual_pipes[2][2] == 700 and edges == {2: (1, 2)}
    checks.load('all matching pipes occupied', graph([1, 2, 3], [(1, 2)]),
                '10\n2\n3\n500\nExtra\n1\n0\n0\n', contains=('Создана труба, ID: 2', 'труба ID=2.'))

    checks.load('invalid endpoints and diameter', base,
                '10\nabc\n999\n-1\n1\n1\n999\n2\n0\n499\n600\n800\n1000x\n700.0\nnan\n1401\n500\n0\n',
                contains=('Объект с таким ID не найден', 'с самой собой', 'Допустимые диаметры', 'труба ID=1.'))
    for name, commands in (('cancel inlet', '10\n0\n'), ('cancel outlet', '10\n1\n0\n')):
        _, files = checks.load(name, base, commands + '6\nout.txt\n0\n', excludes=('Соединение создано',))
        assert saved(files['out.txt'])[2] == {}
    for commands in ('10\n', '10\n1\n', '10\n1\n2\n', '10\n1\n2\n500\n',
                     '10\n1\n2\n500\nNew\n', '10\n1\n2\n500\nNew\n1\n'):
        checks.load('EOF during connection ' + repr(commands), data([], stations, []), commands,
                    excludes=('Соединение создано',))

    connected = graph([1, 2, 3, 4], [(1, 2), (2, 3), (4, 2)])
    _, files = checks.load('disconnect and reuse', connected,
                          '13\n1\n10\n3\n4\n500\n6\nout.txt\n0\n',
                          contains=('Труба отсоединена, ID: 1', 'труба ID=1.'), excludes=('Создайте трубу',))
    assert saved(files['out.txt'])[2] == {1: (3, 4), 2: (2, 3), 3: (4, 2)}
    checks.load('disconnect free pipe', graph([1, 2], [(1, 2)], [(2, 'Spare', 1, 500, 0)]),
                '13\n999\n2\n13\n0\n0\n', contains=('Эта труба уже свободна', 'Объект с таким ID не найден'))
    _, files = checks.load('delete connected pipe', connected, '9\n1\n2\n6\nout.txt\n0\n')
    actual_pipes, _, edges = saved(files['out.txt'])
    assert 2 not in actual_pipes and edges == {1: (1, 2), 3: (4, 2)}
    _, files = checks.load('delete station frees all incident pipes', connected,
                          '9\n2\n2\n10\n1\n3\n500\n6\nout.txt\n0\n', contains=('труба ID=1.',))
    actual_pipes, actual_stations, edges = saved(files['out.txt'])
    assert len(actual_pipes) == 3 and 2 not in actual_stations and edges == {1: (1, 3)}
    _, files = checks.load('bulk delete connected pipes', connected,
                          '8\n1\n1\nPipe\n3\n1\n6\nout.txt\n0\n', contains=('Обработано труб: 3',))
    actual_pipes, actual_stations, edges = saved(files['out.txt'])
    assert actual_pipes == {} and edges == {} and len(actual_stations) == 4
    _, files = checks.load('bulk repair keeps network', connected,
                          '8\n1\n2\n0\n1\n1\n6\nout.txt\n12\n0\n', contains=('Обработано труб: 3',))
    actual_pipes, _, edges = saved(files['out.txt'])
    assert len(edges) == 3 and all(p[3] == 1 for p in actual_pipes.values())
    _, files = checks.load('edit objects keeps network', connected,
                          '4\n1\n1\n5\n1\n1\n6\nout.txt\n0\n')
    actual_pipes, actual_stations, edges = saved(files['out.txt'])
    assert actual_pipes[1][3] == 1 and actual_stations[1][2] == 3 and len(edges) == 3

    output, files = checks.load('save load roundtrip', connected,
                               '6\nсеть с пробелами.txt\n9\n2\n2\n7\nсеть с пробелами.txt\n6\nout.txt\n12\n0\n')
    assert saved(files['out.txt']) == saved(files['сеть с пробелами.txt'])
    valid_order(output, [1, 2, 3, 4], [(1, 2), (2, 3), (4, 2)])
    output, files = checks.load('failed open preserves network', connected,
                               '7\nmissing.txt\n6\nout.txt\n0\n', contains=('Ошибка загрузки',))
    assert len(saved(files['out.txt'])[2]) == 3
    checks.load('protect input log', connected, '6\nlog.txt\n6\n./log.txt\n0\n',
                contains=('нельзя перезаписать журнал',))
    checks.load('save open failure', base, '6\nno_directory/out.txt\n0\n', contains=('Ошибка сохранения',))

    for fixture in (data(pipes, stations), data(pipes, stations).rstrip('\n'), base.rstrip('\n')):
        output, files = checks.load('legacy or final line without newline', fixture, '6\nout.txt\n12\n0\n')
        assert len(saved(files['out.txt'])[0]) == 5 and saved(files['out.txt'])[2] == {}
        valid_order(output, range(1, 5), [])
    legacy = (Path(__file__).resolve().parents[2] / 'examples/lab2/both.txt').read_text(encoding='utf-8')
    checks.load('real lab2 example', legacy, '3\n11\n12\n0\n', contains=('Труба Северная 1', 'КС Южная'))
    _, files = checks.run('legacy load replaces network', '7\nconnected.txt\n7\nlegacy.txt\n6\nout.txt\n0\n',
                         {'connected.txt': connected, 'legacy.txt': data(pipes, stations)})
    assert saved(files['out.txt'])[2] == {}

    invalid = {
        'unknown header': data(pipes, stations) + 'NETWORK_V2\n0\n',
        'missing edge count': data(pipes, stations) + 'NETWORK_V1\n',
        'negative edge count': data(pipes, stations) + 'NETWORK_V1\n-1\n',
        'excessive edge count': data(pipes, stations) + 'NETWORK_V1\n999999999\n',
        'missing pipe': data(pipes, stations, [(999, 1, 2)]),
        'missing inlet': data(pipes, stations, [(1, 999, 2)]),
        'missing outlet': data(pipes, stations, [(1, 1, 999)]),
        'self edge': data(pipes, stations, [(1, 1, 1)]),
        'repeated pipe': data(pipes, stations, [(1, 1, 2), (1, 2, 3)]),
        'negative pipe': data(pipes, stations, [(-1, 1, 2)]),
        'zero inlet': data(pipes, stations, [(1, 0, 2)]),
        'extra text': base + 'extra\n',
        'extra blank line': base + '\n',
        'truncated connection': connected[:-2],
        'bad diameter': data([(1, 'Bad', 1, 600, 0)], stations, []),
        'duplicate station': data([], [stations[0], stations[0]], []),
        'duplicate pipe': data([pipes[0], pipes[0]], stations, []),
        'invalid station ID': data([], [(2147483647, 'Bad', 1, 0, 1)], []),
        'invalid numeric data': data([(1, 'Bad', 'nan', 500, 0)], stations, []),
        'empty file': '',
    }
    for name, fixture in invalid.items():
        _, files = checks.run('reject ' + name,
                             '7\nvalid.txt\n7\nbad.txt\n6\nout.txt\n0\n',
                             {'valid.txt': connected, 'bad.txt': fixture},
                             contains=('Ошибка загрузки', 'Объекты и сеть очищены'))
        assert saved(files['out.txt']) == ({}, {}, {})

    _, files = checks.load('monotonic IDs after deletion', graph([10, 30], [(10, 30)]),
                          '9\n1\n1\n9\n2\n30\n2\nNew station\n1\n0\n1\n10\n10\n31\n500\nNew pipe\n1\n0\n6\nout.txt\n0\n')
    actual_pipes, actual_stations, edges = saved(files['out.txt'])
    assert set(actual_pipes) == {2} and set(actual_stations) == {10, 31} and edges == {2: (10, 31)}

    for diameter in (500, 700, 1000, 1400):
        _, files = checks.run(f'manual pipe {diameter}',
                             f'1\nManual\n1\n{diameter}\n0\n6\nout.txt\n0\n')
        assert saved(files['out.txt'])[0][1][2] == diameter
    checks.run('manual diameter validation', '1\nManual\n1\n100\n600\n999\n1401\n500\n0\n0\n',
               contains=('Допустимые диаметры', 'Ошибка ввода'))
    checks.run('raw log blank whitespace unicode', '\n \t\nошибка\n14\n-1\n3x\n  +3 \t\n0\n',
               contains=('Ошибка ввода',))
    for typed in ('', '0', 'bad \t', '1\nНеполная труба'):
        checks.run('console EOF ' + repr(typed), typed)

    root = Path(__file__).resolve().parents[2]
    dag = (root / 'examples/lab3/dag.txt').read_text(encoding='utf-8')
    output, _ = checks.load('documented DAG example', dag, '11\n12\n0\n')
    assert 'Топологический порядок КС: 1 5 2 3 4' in output
    valid_order(output, [1, 2, 3, 4, 5], [(1, 2), (1, 3), (2, 4), (3, 4)])
    cycle = (root / 'examples/lab3/cycle.txt').read_text(encoding='utf-8')
    output, _ = checks.load('cycle resolved by disconnecting pipe', cycle,
                           '12\n13\n5\n12\n0\n', contains=('в сети есть цикл', 'Труба отсоединена, ID: 5'))
    # There must be exactly one full order, produced after breaking the cycle.
    assert 'в сети есть цикл' in output
    valid_order(output.split('Труба отсоединена, ID: 5', 1)[1],
                [1, 2, 3, 4, 5], [(1, 2), (1, 3), (2, 4), (3, 4)])
    limit = data([(2147483646, 'Last pipe', 1, 500, 0)], stations, [(2147483646, 1, 2)])
    _, files = checks.load('ID exhaustion does not create edge', limit,
                          '10\n2\n3\n500\n6\nout.txt\n0\n', contains=('Закончились свободные ID труб',))
    assert saved(files['out.txt'])[2] == {2147483646: (1, 2)}

    fixed_graphs = [
        ([1], []), ([1, 2, 3], []), ([1, 2, 3, 4], [(1, 2), (1, 3), (2, 4), (3, 4)]),
        ([1, 2, 3, 4, 5, 6], [(5, 2), (2, 1), (4, 3)]),
        ([1, 2, 3], [(1, 2), (1, 2), (2, 3)]),
        ([1, 2], [(1, 2), (2, 1)]),
        ([1, 2, 3, 4, 5], [(1, 2), (3, 4), (4, 5), (5, 3)]),
    ]
    rng = random.Random(20261118)
    generated = []
    for index in range(48):
        ids = rng.sample(range(1, 1000), rng.randint(2, 18))
        if index % 2 == 0:
            links = [(a, b) for i, a in enumerate(ids) for b in ids[i + 1:] if rng.random() < 0.2]
        else:
            links = [(a, b) for a in ids for b in ids if a != b and rng.random() < 0.16]
        if links and index % 3 == 0:
            links.append(links[0])
        generated.append((ids, links))
    for index, (ids, links) in enumerate(fixed_graphs + generated):
        output, _ = checks.load(f'graph oracle {index}', graph(ids, links), '12\n0\n')
        if has_cycle(ids, links):
            assert 'в сети есть цикл' in output and 'Топологический порядок КС:' not in output
        else:
            valid_order(output, ids, links)
    ids = list(range(1, 2001))
    links = [(i, i + 1) for i in ids[:-1]]
    output, _ = checks.load('long chain without recursion', graph(ids, links), '12\n0\n')
    valid_order(output, ids, links)
    output, _ = checks.load('long cycle', graph(ids, links + [(2000, 1)]), '12\n0\n')
    assert 'в сети есть цикл' in output and 'Топологический порядок КС:' not in output

    with tempfile.TemporaryDirectory(prefix='lab3_check_') as temp:
        cwd = Path(temp)
        for typed in (b'invalid\n0\n', b'11\n0\n'):
            result = subprocess.run([checks.exe], input=typed, cwd=cwd, capture_output=True, timeout=10)
            assert result.returncode == 0
        assert (cwd / 'log.txt').read_bytes().replace(b'\r\n', b'\n') == b'invalid\n0\n11\n0\n'
        checks.count += 1
    with tempfile.TemporaryDirectory(prefix='lab3_check_') as temp:
        cwd = Path(temp)
        (cwd / 'log.txt').mkdir()
        result = subprocess.run([checks.exe], input=b'0\n', cwd=cwd, capture_output=True, timeout=10)
        assert result.returncode == 1 and 'Не удалось открыть log.txt' in result.stderr.decode('utf-8')
        checks.count += 1
    print(f'{checks.exe}: {checks.count} scenarios passed')


if __name__ == '__main__':
    default = Path(__file__).resolve().parents[2] / 'x64/Debug/anisimov_Lab3.exe'
    main(Path(sys.argv[1]) if len(sys.argv) > 1 else default)
