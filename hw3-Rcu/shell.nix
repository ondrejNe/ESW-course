let
  sources = import ./npins;
  pkgs = import sources.nixpkgs {};
in
with pkgs;
mkShell {
	buildInputs = [
		pkg-config
		meson
		ninja
		liburcu
		linuxPackages.perf
	];
}
