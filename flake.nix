{
    description = "A very basic flake";

    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-25.11";
        flake-parts.url = "github:hercules-ci/flake-parts";
    };

    outputs = { flake-parts, ...} @inputs:
        flake-parts.lib.mkFlake { inherit inputs; } {
            # Linux x86 and ARM as well as MacOS x86 and ARM
            systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
            perSystem = {pkgs, ...}: {
                devShells.default = pkgs.mkShell {
                    buildInputs = with pkgs; [
                        gcc-arm-embedded
                        openocd
                        stlink
                        cmake
                        ninja
                    ];
                };
            };
        };
}
