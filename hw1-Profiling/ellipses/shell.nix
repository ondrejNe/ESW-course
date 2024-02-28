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
		boost
		(opencv.override { enableGtk3 = true; })

		hyperfine

		linuxKernel.packages.linux_latest_libre.perf
		hotspot

		valgrind
		libsForQt5.kcachegrind
		graphviz
	];

  # Meson is no longer able to pick up Boost automatically.
  # https://github.com/NixOS/nixpkgs/issues/86131
  BOOST_INCLUDEDIR = "${lib.getDev boost}/include";
  BOOST_LIBRARYDIR = "${lib.getLib boost}/lib";
}
