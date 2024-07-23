const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});


    const glfw = getGlfw(b, optimize, target);

    const exe = b.addExecutable(.{
        .name = "rts",
        .target = target,
        .optimize = optimize,
    });
    exe.linkLibCpp();
    exe.addCSourceFiles(.{
        .files = &.{"src/main.cpp"},
    });
    exe.addLibraryPath(.{.path="../../zig-out/lib"});
    exe.linkLibrary(glfw);
    exe.linkSystemLibrary("nhw");
    const vulkanName = if(target.result.os.tag == .windows) "vulkan-1" else "vulkan";
    exe.linkSystemLibrary(vulkanName);
    exe.addIncludePath(.{.path="../../src/"});
    exe.addLibraryPath(.{.path="../../thirdparty/"});
    b.installArtifact(exe);

    // Run example
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

fn getGlfw(
    b: *std.Build,
    optimize: std.builtin.OptimizeMode,
    target: std.Build.ResolvedTarget,
) *std.Build.Step.Compile {
    const glfw = b.dependency("glfw", .{
        .optimize = optimize,
        .target = target,
    });

    return glfw.artifact("glfw");
}
