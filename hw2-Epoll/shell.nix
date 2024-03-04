with (import <nixpkgs> {});
mkShell {
	buildInputs = [
		pkg-config
		meson
		ninja
		netcat
	];
}
