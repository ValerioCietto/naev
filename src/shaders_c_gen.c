#include <stdio.h>

typedef struct Shader_ {
   const char *name;
   const char *vs_path;
   const char *fs_path;
   const char *attributes[16];
   const char *uniforms[16];
} Shader;

const Shader shaders[] = {
   {
      .name = "circle",
      .vs_path = "circle.vert",
      .fs_path = "circle.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "color", "radius"}
   },
   {
      .name = "circle_filled",
      .vs_path = "circle.vert",
      .fs_path = "circle_filled.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "color", "radius"}
   },
   {
      .name = "solid",
      .vs_path = "solid.vert",
      .fs_path = "solid.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "color"}
   },
   {
      .name = "smooth",
      .vs_path = "smooth.vert",
      .fs_path = "smooth.frag",
      .attributes = {"vertex", "vertex_color"},
      .uniforms = {"projection"}
   },
   {
      .name = "texture",
      .vs_path = "texture.vert",
      .fs_path = "texture.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "color", "tex_mat"}
   },
   {
      .name = "texture_interpolate",
      .vs_path = "texture.vert",
      .fs_path = "texture_interpolate.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "color", "tex_mat", "sampler1", "sampler2", "inter"}
   },
   {
      .name = "nebula",
      .vs_path = "nebula.vert",
      .fs_path = "nebula.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "color", "center", "radius"}
   },
   {
      .name = "stars",
      .vs_path = "stars.vert",
      .fs_path = "stars.frag",
      .attributes = {"vertex", "brightness"},
      .uniforms = {"projection", "star_xy", "wh", "xy"}
   },
   {
      .name = "font",
      .vs_path = "font.vert",
      .fs_path = "font.frag",
      .attributes = {"vertex", "tex_coord"},
      .uniforms = {"projection", "color"}
   },
   {
      .name = "beam",
      .vs_path = "beam.vert",
      .fs_path = "beam.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "color", "tex_mat"}
   },
   {
      .name = "tk",
      .vs_path = "tk.vert",
      .fs_path = "tk.frag",
      .attributes = {"vertex"},
      .uniforms = {"projection", "c", "dc", "lc", "oc", "wh", "corner_radius"}
   },
};

const int nshaders = sizeof(shaders) / sizeof(Shader);

static void generate_h_file(FILE *f) {
   int i, j;

   fprintf(f, "/* FILE GENERATED BY shader_c_gen.c */\n\n");

   fprintf(f, "#ifndef SHADER_GEN_C_H\n");
   fprintf(f, "#define SHADER_GEN_C_H\n");

   fprintf(f, "#include \"opengl.h\"\n\n");

   fprintf(f, "typedef struct Shaders_ {\n");
   for (i = 0; i < nshaders; i++) {
      fprintf(f, "   struct {\n");

      fprintf(f, "      GLuint program;\n");

      for (j = 0; shaders[i].attributes[j] != NULL; j++) {
         fprintf(f, "      GLuint %s;\n", shaders[i].attributes[j]);
      }

      for (j = 0; shaders[i].uniforms[j] != NULL; j++) {
         fprintf(f, "      GLuint %s;\n", shaders[i].uniforms[j]);
      }

      fprintf(f, "   } %s;\n", shaders[i].name);
   }
   fprintf(f, "} Shaders;\n\n");

   fprintf(f, "extern Shaders shaders;\n\n");

   fprintf(f, "void shaders_load (void);\n");
   fprintf(f, "void shaders_unload (void);\n");

   fprintf(f, "#endif\n");
}

static void generate_c_file(FILE *f) {
   int i, j;

   fprintf(f, "/* FILE GENERATED BY shader_c_gen.c */\n\n");

   fprintf(f, "#include <string.h>\n");
   fprintf(f, "#include \"shaders.gen.h\"\n");
   fprintf(f, "#include \"opengl_shader.h\"\n\n");

   fprintf(f, "Shaders shaders;\n\n");

   fprintf(f, "void shaders_load (void) {\n");
   for (i = 0; i < nshaders; i++) {
      fprintf(f, "   shaders.%s.program = gl_program_vert_frag(\"%s\", \"%s\");\n",
            shaders[i].name,
            shaders[i].vs_path,
            shaders[i].fs_path);
      for (j = 0; shaders[i].attributes[j] != NULL; j++) {
         fprintf(f, "   shaders.%s.%s = glGetAttribLocation(shaders.%s.program, \"%s\");\n",
            shaders[i].name,
            shaders[i].attributes[j],
            shaders[i].name,
            shaders[i].attributes[j]);
      }

      for (j = 0; shaders[i].uniforms[j] != NULL; j++) {
         fprintf(f, "   shaders.%s.%s = glGetUniformLocation(shaders.%s.program, \"%s\");\n",
            shaders[i].name,
            shaders[i].uniforms[j],
            shaders[i].name,
            shaders[i].uniforms[j]);
      }

      if (i != nshaders - 1) {
         fprintf(f, "\n");
      }
   }
   fprintf(f, "}\n\n");

   fprintf(f, "void shaders_unload (void) {\n");
   for (i = 0; i < nshaders; i++) {
      fprintf(f, "   glDeleteProgram(shaders.%s.program);\n", shaders[i].name);

   }
   fprintf(f, "   memset(&shaders, 0, sizeof(shaders));\n");
   fprintf(f, "}\n");

}

int main( int argc, char** argv ) {
   (void) argc;
   (void) argv;
   FILE *shaders_gen_h, *shaders_gen_c;
   shaders_gen_h = fopen("shaders.gen.h", "w");
   shaders_gen_c = fopen("shaders.gen.c", "w");
   generate_h_file( shaders_gen_h );
   generate_c_file( shaders_gen_c );
   fclose( shaders_gen_h );
   fclose( shaders_gen_c );
   return 0;
}
