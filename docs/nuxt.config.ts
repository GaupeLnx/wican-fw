export default defineNuxtConfig({
  // https://github.com/nuxt-themes/docus
  extends: ['@nuxt-themes/docus'],
  app: {
    baseURL: '/wican-fw/' 
  },

  devtools: { enabled: false },
  nitro: {
    prerender: {
      failOnError: false
    }
  },
  modules: [
    // Remove it if you don't use Plausible analytics
    // https://github.com/nuxt-modules/plausible
    //'@nuxtjs/plausible'
  ],

  compatibilityDate: '2024-09-07'
})
