import json
from pathlib import Path


def path_literal(path: Path | str) -> str:
    return json.dumps(str(path))


def round_trip_source(text_path: Path, bytes_path: Path) -> str:
    return f"""
main(): Void {{
  write_file({path_literal(text_path)}, "λ-data");
  print(read_file({path_literal(text_path)}));
  mut data: Bytes = bytes();
  data = byte_append(data, 0);
  data = byte_append(data, 255);
  data = byte_append(data, 65);
  write_bytes({path_literal(bytes_path)}, data);
  print(read_bytes({path_literal(bytes_path)}));
}}
"""


def read_source(service: str, path: Path | str) -> str:
    return f"main(): Void {{ print({service}({path_literal(path)})); }}"


def write_source(service: str, path: Path | str) -> str:
    value = '"data"' if service == "write_file" else 'utf8_encode("data")'
    return f"main(): Void {{ {service}({path_literal(path)}, {value}); }}"


def nul_path_source(service: str) -> str:
    value = '"data"' if service == "write_file" else 'utf8_encode("data")'
    if service.startswith("read"):
        return f'main(): Void {{ print({service}("invalid\\u0000path")); }}'
    return f'main(): Void {{ {service}("invalid\\u0000path", {value}); }}'
